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

#ifndef SYNTHESIS_H
#define SYNTHESIS_H

#include <memory>

#include <QString>



namespace GS {

namespace VTMControlModel {
class Controller;
class Model;
}

struct Synthesis {
	QString projectDir;
	std::unique_ptr<VTMControlModel::Controller> vtmController;

	Synthesis();
	~Synthesis();

	void clear();
	void setup(const QString& newProjectDir, VTMControlModel::Model* model);
};

} // namespace GS

#endif // SYNTHESIS_H
