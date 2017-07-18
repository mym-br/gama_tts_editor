/***************************************************************************
 *  Copyright 2008, 2014 Marcelo Y. Matuda                                 *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
/***************************************************************************
 *  The code that handles the communication with the JACK server was
 *  based on simple_client.c from JACK 1.9.10.
 ***************************************************************************/

#include "Audio.h"

#include <cassert>
#include <cstdlib>
#include <iostream>

#include "ProgramConfiguration.h"



namespace {

using namespace GS;

const char* CLIENT_NAME = "gama_tts_interactive";



extern "C" {

/*******************************************************************************
 * The process callback for this JACK application is called in a
 * special realtime thread once for each audio cycle.
 */
int
jack_process_callback(jack_nframes_t nframes, void* arg)
{
	try {
		Audio::Processor* p = static_cast<Audio::Processor*>(arg);
		return p->process(nframes);
	} catch (std::exception& exc) {
		std::cerr << "[Audio/jack_process_callback] Caught exception: " << exc.what() << '.' << std::endl;
		return 1;
	}
}

/*******************************************************************************
 * JACK calls this function if the server ever shuts down or
 * decides to disconnect the client.
 */
void
jack_shutdown_callback(void* /*arg*/)
{
	std::cout << "[Audio] jack_shutdown_callback()" << std::endl;
}

} /* extern "C" */

} /* namespace */

//==============================================================================

namespace GS {

/*******************************************************************************
 * Constructor.
 */
Audio::Processor::Processor()
		: outputPort_{}
		, outputScale_{}
		, vocalTractModel_{}
		, parameterRingbuffer_{}
		, analysisRingbuffer_{}
{
}

/*******************************************************************************
 * Destructor.
 */
Audio::Processor::~Processor()
{
}

/*******************************************************************************
 *
 */
void
Audio::Processor::reset(jack_port_t* outputPort, ProgramConfiguration& configuration,
			JackRingbuffer& parameterRingbuffer, JackRingbuffer& analysisRingbuffer)
{
	vocalTractModel_ = VTM::VocalTractModel::getInstance(*configuration.vtmData, true);
	outputPort_ = outputPort;
	parameterRingbuffer_ = &parameterRingbuffer;
	analysisRingbuffer_ = &analysisRingbuffer;
}

/*******************************************************************************
 *
 */
int
Audio::Processor::process(jack_nframes_t nframes)
{
	if (!vocalTractModel_) {
		return 1; // error
	}

//	jack_transport_state_t ts = jack_transport_query(client_, 0);
//	if (ts == JackTransportRolling) {
//	} else if (ts == JackTransportStopped) {
//	}

	jack_default_audio_sample_t* out = static_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(outputPort_, nframes));
	const std::size_t sampleSize = sizeof(jack_default_audio_sample_t);

	const std::size_t n = vocalTractModel_->getOutputSamples(nframes, out);

	// Send data to analysis.
	for (std::size_t i = 0; i < n; ++i) {
		if (analysisRingbuffer_ && analysisRingbuffer_->writeSpace() >= sampleSize) {
#ifndef NDEBUG
			const std::size_t bytesWritten =
#endif
			analysisRingbuffer_->write(reinterpret_cast<const char*>(out + i), sampleSize);
			assert(bytesWritten == sampleSize);
		} else {
			//std::cerr << "[Audio::Processor::process] Analysis buffer full." << std::endl;
			break;
		}
	}

	if (n == nframes) return 0; // JACK does not need more samples

	// JACK needs more samples.

	// Read parameters from ringbuffer, and send them to vocal tract model.
	const size_t elementSize = sizeof(VocalTractModelParameterValue);
	VocalTractModelParameterValue pv;
	while (parameterRingbuffer_->readSpace() >= elementSize) {
#ifndef NDEBUG
		size_t bytesRead =
#endif
		parameterRingbuffer_->read(reinterpret_cast<char*>(&pv), elementSize);
		assert(bytesRead == elementSize);
		vocalTractModel_->loadSingleInput(pv);
	}

	vocalTractModel_->synthesizeSamples(nframes - n);
	const std::size_t n2 = vocalTractModel_->getOutputSamples(nframes - n, out + n);
	assert(n2 == nframes - n);

	// Send data to analysis.
	jack_default_audio_sample_t* out2 = out + n;
	for (std::size_t i = 0; i < n2; ++i) {
		if (analysisRingbuffer_ && analysisRingbuffer_->writeSpace() >= sampleSize) {
#ifndef NDEBUG
			const std::size_t bytesWritten =
#endif
			analysisRingbuffer_->write(reinterpret_cast<const char*>(out2 + i), sampleSize);
			assert(bytesWritten == sampleSize);
		} else {
			//std::cerr << "[Audio::Processor::process] Analysis buffer full (2)." << std::endl;
			break;
		}
	}

	return 0;
}

/*******************************************************************************
 * Constructor.
 */
Audio::Audio(ProgramConfiguration& configuration)
		: state_{STOPPED}
		, configuration_{configuration}
		, processor_{}
		, parameterRingbuffer_{std::make_unique<JackRingbuffer>(PARAMETER_RINGBUFFER_SIZE * sizeof(VocalTractModelParameterValue))}
		// +1 because the ringbuffer keeps at least one position open.
		, analysisRingbuffer_{std::make_unique<JackRingbuffer>(MAX_NUM_SAMPLES_FOR_ANALYSIS * sizeof(jack_default_audio_sample_t) + 1)}
		, jackClient_{}
		, sampleRate_{}
{
}

/*******************************************************************************
 * Starts the connection to the JACK server.
 *
 * Preconditions:
 * - The parameter ringbuffer must be filled with a complete set of parameter
 *   values.
 */
void
Audio::start()
{
	if (state_ == STARTED) {
		if (!stop()) {
			return;
		}
	}

	auto newJackClient = std::make_unique<JackClient>(CLIENT_NAME);

	newJackClient->setProcessCallback(jack_process_callback, &processor_);

	newJackClient->setShutdownCallback(jack_shutdown_callback, nullptr);

	jack_port_t* outputPort = newJackClient->registerPort("output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

	jack_nframes_t jackSampleRate = newJackClient->getSampleRate();
	sampleRate_ = jackSampleRate;

	// Prepare the configuration.
	const float outputRate = static_cast<float>(jackSampleRate);
	configuration_.setOutputRate(outputRate);
	std::cout << "Output sample rate: " << outputRate << std::endl;

	// Prepare the audio processor.
	processor_.reset(outputPort, configuration_, *parameterRingbuffer_, *analysisRingbuffer_);

	newJackClient->activate();

	// Connect the ports. You can't do this before the client is
	// activated, because we can't make connections to clients
	// that aren't running. Note the confusing (but necessary)
	// orientation of the driver backend ports: playback ports are
	// "input" to the backend, and capture ports are "output" from it.
	JackPorts ports;
	newJackClient->getPorts(NULL, NULL, JackPortIsPhysical | JackPortIsInput, ports);
	if (ports.list == NULL) {
		THROW_EXCEPTION(AudioException, "No physical playback ports.");
	}
	for (size_t i = 0; i < 2 && ports.list[i]; ++i) {
		newJackClient->connect(JackClient::portName(outputPort), ports.list[i]);
	}

	jackClient_.reset();
	jackClient_ = std::move(newJackClient);
	state_ = STARTED;
	std::cout << "Audio started." << std::endl;
}

/*******************************************************************************
 * Stops the connection to the JACK server.
 *
 * Returns false if an error ocurred.
 */
bool
Audio::stop()
{
	if (state_ == STOPPED) return true;

	jackClient_.reset();

	state_ = STOPPED;
	std::cout << "Audio stopped." << std::endl;
	return true;
}

} /* namespace GS */
