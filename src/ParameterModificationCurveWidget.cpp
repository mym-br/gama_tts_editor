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

#include "ParameterModificationCurveWidget.h"

#include <QPainter>



namespace GS {

ParameterModificationCurveWidget::ParameterModificationCurveWidget(QWidget* parent)
		: QWidget{parent}
{
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
}

void
ParameterModificationCurveWidget::paintEvent(QPaintEvent* /*event*/)
{
	QPainter painter(this);

	const int xEnd = width() - 1;
	const int yEnd = height() - 1;
	painter.drawLine(0, 0, 0, yEnd);
	painter.drawLine(0, 0, xEnd, 0);
	painter.drawLine(xEnd, 0, xEnd, yEnd);
	painter.drawLine(0, yEnd, xEnd, yEnd);
}

} // namespace GS
