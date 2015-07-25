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

#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "portaudio.h"
#include "portaudiocpp/AutoSystem.hxx"



namespace GS {

class AudioPlayer {
public:
	AudioPlayer();

	std::vector<float>& buffer() { return buffer_; }
	void getOutputDeviceList(std::vector<std::string>& deviceNameList, int& defaultDeviceIndex);
	void play(double sampleRate, int outputDeviceIndex);

	static std::mutex bufferMutex;
private:
	AudioPlayer(const AudioPlayer&) = delete;
	AudioPlayer& operator=(const AudioPlayer&) = delete;

	int callback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
			const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);

	std::vector<float> buffer_;
	unsigned int bufferIndex_;
};

} // namespace GS

#endif // AUDIO_PLAYER_H
