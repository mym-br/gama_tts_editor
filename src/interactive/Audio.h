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

#ifndef AUDIO_H_
#define AUDIO_H_

#include <memory>
#include <vector>

#include "Exception.h"
#include "JackClient.h"
#include "JackRingbuffer.h"
#include "VocalTractModel.h"



namespace GS {

struct AudioException : public virtual Exception {};

struct ProgramConfiguration;

class Audio {
public:
	enum {
		PARAMETER_RINGBUFFER_SIZE = 32,
		MAX_NUM_SAMPLES_FOR_ANALYSIS = 65536
	};
	enum AudioState {
		STARTED,
		STOPPED
	};

	class Processor {
	public:
		Processor();
		~Processor();

		void reset(jack_port_t* outputPort, ProgramConfiguration& configuration,
				JackRingbuffer& parameterRingbuffer, JackRingbuffer& analysisRingbuffer);
		int process(jack_nframes_t nframes);
	private:
		jack_port_t* outputPort_;
		float outputScale_;
		std::unique_ptr<VTM::VocalTractModel> vocalTractModel_;
		JackRingbuffer* parameterRingbuffer_;
		JackRingbuffer* analysisRingbuffer_;
	};

	Audio(ProgramConfiguration& configuration);

	void start();
	bool stop();

	JackRingbuffer& parameterRingbuffer() { return *parameterRingbuffer_; }
	JackRingbuffer& analysisRingbuffer() { return *analysisRingbuffer_; }
	unsigned int sampleRate() const { return sampleRate_; }
private:
	Audio(const Audio&) = delete;
	Audio& operator=(const Audio&) = delete;

	AudioState state_;
	ProgramConfiguration& configuration_;
	Processor processor_; // must be accessed only by the JACK thread
	std::unique_ptr<JackRingbuffer> parameterRingbuffer_;
	std::unique_ptr<JackRingbuffer> analysisRingbuffer_;
	std::unique_ptr<JackClient> jackClient_;
	unsigned int sampleRate_;
};

} /* namespace GS */

#endif /* AUDIO_H_ */
