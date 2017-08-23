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

#ifndef PARAMETER_MODIFICATION_CURVE_WIDGET_H
#define PARAMETER_MODIFICATION_CURVE_WIDGET_H

#include <QWidget>

namespace GS {

class ParameterModificationCurveWidget : public QWidget {
	Q_OBJECT
public:
	explicit ParameterModificationCurveWidget(QWidget* parent=nullptr);
protected:
	virtual void paintEvent(QPaintEvent* event);
};

} // namespace GS

#endif // PARAMETER_MODIFICATION_CURVE_WIDGET_H
