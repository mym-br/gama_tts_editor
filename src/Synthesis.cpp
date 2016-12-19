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

#include "Synthesis.h"

#include "Controller.h"
#include "en/phonetic_string_parser/PhoneticStringParser.h"
#include "en/text_parser/TextParser.h"



namespace GS {

Synthesis::Synthesis()
{
}

Synthesis::~Synthesis()
{
}

void
Synthesis::clear()
{
	phoneticStringParser.reset(0);
	textParser.reset(0);
	vtmController.reset(0);
	projectDir.clear();
}

void
Synthesis::setup(const QString& newProjectDir, VTMControlModel::Model* model)
{
	if (model == nullptr) {
		clear();
		return;
	}
	try {
		projectDir = newProjectDir;
		const std::string configDirPath = projectDir.toStdString();
		vtmController = std::make_unique<VTMControlModel::Controller>(configDirPath.c_str(), *model);
		const VTMControlModel::Configuration& vtmControlConfig = vtmController->vtmControlModelConfiguration();
		textParser = std::make_unique<En::TextParser>(configDirPath.c_str(),
							vtmControlConfig.dictionary1File,
							vtmControlConfig.dictionary2File,
							vtmControlConfig.dictionary3File);
		phoneticStringParser = std::make_unique<En::PhoneticStringParser>(configDirPath.c_str(), *vtmController);
	} catch (...) {
		clear();
		throw;
	}
}

} // namespace GS
