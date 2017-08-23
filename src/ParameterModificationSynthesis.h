/***************************************************************************
 *  Copyright 2017 Marcelo Y. Matuda                                       *
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

#ifndef PARAMETER_MODIFICATION_SYNTHESIS_H
#define PARAMETER_MODIFICATION_SYNTHESIS_H

#include <memory>
#include <vector>

#include "JackClient.h"
#include "JackRingbuffer.h"
//#include "MovingAverageFilter.h"



namespace GS {

class ConfigurationData;
namespace VTM {
class VocalTractModel;
}
namespace VTMControlModel {
class Controller;
}

class ParameterModificationSynthesis {
public:
	struct Modification {
		unsigned int parameter;
		unsigned int operation; // 0:add 1:multiply
		float value;
	};

	class Processor {
	public:
		Processor(
			unsigned int numberOfParameters,
			JackRingbuffer* parameterRingbuffer,
			const ConfigurationData& vtmConfigData,
			unsigned int controlSteps);
		~Processor();

		// Called only by the JACK thread.
		int process(jack_nframes_t nframes);

		// These functions can be called by the main thread only when the JACK thread is not running.
		void resetData(const std::vector<std::vector<float>>& paramList);
		bool validData() const;
		void prepareSynthesis(jack_port_t* jackOutputPort, float gain);

		// Can be called by any thread.
		bool running() const;
	private:
		unsigned int numParameters_;
		jack_port_t* outputPort_;
		std::size_t vtmBufferPos_;
		JackRingbuffer* parameterRingbuffer_;
		std::vector<std::vector<float>> paramList_;
		std::vector<std::vector<float>> modifiedParamList_;
		std::unique_ptr<VTM::VocalTractModel> vocalTractModel_;
		std::vector<float> currentParam_;
		std::vector<float> delta_;
		float gain_;
		unsigned int stepIndex_;
		unsigned int paramSetIndex_;
		unsigned int controlSteps_;
		Modification modif_;
	};

	ParameterModificationSynthesis(
		unsigned int numberOfParameters,
		double vtmInternalSampleRate,
		double controlRate,
		const ConfigurationData& vtmConfigData);
	~ParameterModificationSynthesis() {}

	void startSynthesis(float gain);

	// Returns false when there are no more data to process.
	bool modifyParameter(
			unsigned int parameter,
			unsigned int operation, // 0:add 1:multiply
			float value);

	void resetData(const std::vector<std::vector<float>>& paramList);
private:
	enum {
		PARAMETER_RINGBUFFER_SIZE = 8
	};

	void stop();

	std::unique_ptr<JackRingbuffer> parameterRingbuffer_;
	std::unique_ptr<Processor> processor_; // used by the JACK thread
	std::unique_ptr<JackClient> jackClient_;
};

} // namespace GS

#endif // PARAMETER_MODIFICATION_SYNTHESIS_H
