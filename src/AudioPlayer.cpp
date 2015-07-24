/***************************************************************************
 *  Copyright 2015 Marcelo Y. Matuda                                       *
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

#include "AudioPlayer.h"

#include <sstream>

#include "portaudiocpp/PortAudioCpp.hxx"

#include "Exception.h"



namespace GS {

AudioPlayer::AudioPlayer()
		: bufferIndex_(0)
{
}

void
AudioPlayer::getOutputDeviceList(std::vector<std::string>& deviceNameList, int& defaultDeviceIndex)
{
	portaudio::AutoSystem portaudio;

	deviceNameList.clear();
	defaultDeviceIndex = -1;

	portaudio::System& sys = portaudio::System::instance();

	int i = 0;
	for (auto iter = sys.devicesBegin(); iter != sys.devicesEnd(); ++iter, ++i) {
		std::ostringstream name;
		name << '[' << iter->hostApi().name() << "] " << iter->name();
		deviceNameList.push_back(name.str());

		if (iter->hostApi().typeId() == paJACK && iter->isHostApiDefaultOutputDevice()) {
			defaultDeviceIndex = i;
		}
	}
	if (defaultDeviceIndex < 0) {
		defaultDeviceIndex = sys.defaultOutputDevice().index();
	}
}

void
AudioPlayer::play(double sampleRate, int outputDeviceIndex)
{
	portaudio::AutoSystem portaudio;
	bufferIndex_ = 0;

	//---------------------------------------------------------------------
	// Play the audio.

	portaudio::System& sys = portaudio::System::instance();

	if (outputDeviceIndex >= sys.deviceCount()) {
		THROW_EXCEPTION(IOException, "Invalid device index: " << outputDeviceIndex << '.');
	}
	portaudio::Device& dev = sys.deviceByIndex(outputDeviceIndex);

	portaudio::DirectionSpecificStreamParameters outParams(dev, 2 /* channels */, portaudio::FLOAT32,
						true /* interleaved */, dev.defaultHighOutputLatency(), NULL);

	portaudio::StreamParameters params(portaudio::DirectionSpecificStreamParameters::null(), outParams, sampleRate,
						paFramesPerBufferUnspecified, paClipOff);

	portaudio::MemFunCallbackStream<AudioPlayer> stream(params, *this, &AudioPlayer::callback);

	stream.start();
	while (stream.isActive()) {
		sys.sleep(200 /* ms */);
	}
	stream.stop();
	stream.close();
}

int
AudioPlayer::callback(const void* /*inputBuffer*/, void* outputBuffer, unsigned long framesPerBuffer,
			const PaStreamCallbackTimeInfo* /*timeInfo*/, PaStreamCallbackFlags /*statusFlags*/)
{
	float* out = static_cast<float*>(outputBuffer);
	const unsigned int framesAvailable = buffer_.size() - bufferIndex_;
	const unsigned int numFrames = (framesAvailable > framesPerBuffer) ? framesPerBuffer : framesAvailable;
	for (unsigned int i = 0; i < numFrames; ++i) {
		unsigned int baseIndex = i * 2;
		const float* src = &buffer_[bufferIndex_ + i];
		out[baseIndex]     = *src;
		out[baseIndex + 1] = *src;
	}
	bufferIndex_ += numFrames;
	for (unsigned int i = numFrames; i < framesPerBuffer; ++i) {
		const unsigned int baseIndex = i * 2;
		out[baseIndex]     = 0;
		out[baseIndex + 1] = 0;
	}
	if (bufferIndex_ == buffer_.size()) {
		return paComplete;
	} else {
		return paContinue;
	}
}

} // namespace GS
