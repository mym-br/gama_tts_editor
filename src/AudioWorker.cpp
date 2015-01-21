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

#include "AudioWorker.h"

#include <exception>
#include <string>
#include <vector>



namespace GS {

AudioWorker::AudioWorker(QObject* parent)
		: QObject(parent)
{
}

AudioWorker::~AudioWorker()
{
}

// Slot.
void
AudioWorker::sendOutputDeviceList()
{
	std::vector<std::string> deviceNameList;
	int defaultDeviceIndex;
	player_.getOutputDeviceList(deviceNameList, defaultDeviceIndex);

	QStringList list;
	for (const auto& name : deviceNameList) {
		list.append(name.c_str());
	}
	emit audioOutputDeviceListSent(list, defaultDeviceIndex);
}

// Slot.
void
AudioWorker::playAudioFile(QString filePath, int outputDeviceIndex)
{
	try {
		player_.playFile(filePath.toStdString(), outputDeviceIndex);
	} catch (const std::exception& exc) {
		emit errorOccurred(QString(exc.what()));
	}

	emit finished();
}

} // namespace GS
