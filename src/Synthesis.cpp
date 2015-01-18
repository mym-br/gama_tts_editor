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
	trmController.reset(0);
	projectDir.clear();
}

void
Synthesis::setup(const QString& newProjectDir, TRMControlModel::Model* model)
{
	if (model == nullptr) {
		clear();
		return;
	}
	try {
		projectDir = newProjectDir;
		const std::string configDirPath = projectDir.toStdString();
		trmController.reset(new TRMControlModel::Controller(configDirPath.c_str(), *model));
		textParser.reset(new En::TextParser(configDirPath.c_str()));
		phoneticStringParser.reset(new En::PhoneticStringParser(configDirPath.c_str(), *trmController));
	} catch (...) {
		clear();
		throw;
	}
}

} // namespace GS
