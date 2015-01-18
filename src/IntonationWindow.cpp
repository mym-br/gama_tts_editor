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

#include "IntonationWindow.h"

#include <QDoubleValidator>
#include <QString>

#include "Controller.h"
#include "Synthesis.h"
#include "ui_IntonationWindow.h"



namespace GS {

IntonationWindow::IntonationWindow(QWidget* parent)
		: QWidget(parent)
		, ui_(new Ui::IntonationWindow)
		, model_(nullptr)
		, synthesis_(nullptr)
{
	ui_->setupUi(this);

	connect(ui_->intonationWidget, SIGNAL(pointSelected(double, double, double, double, double)),
		this, SLOT(setPointData(double, double, double, double, double)));
}

IntonationWindow::~IntonationWindow()
{
}

void
IntonationWindow::clear()
{
	synthesis_ = nullptr;
	model_ = nullptr;
}

void
IntonationWindow::setup(TRMControlModel::Model* model, Synthesis* synthesis)
{
	if (model == nullptr || synthesis == nullptr) {
		clear();
		return;
	}

	model_ = model;
	synthesis_ = synthesis;

	ui_->intonationWidget->updateData(&synthesis_->trmController->eventList());
}

// Slot.
void
IntonationWindow::loadIntonationFromEventList()
{
	ui_->intonationWidget->loadIntonationFromEventList();
}

void
IntonationWindow::on_valueLineEdit_editingFinished()
{
	bool ok;
	double value = ui_->valueLineEdit->text().toDouble(&ok);
	if (!ok) {
		ui_->valueLineEdit->clear();
		return;
	}
	ui_->intonationWidget->setSelectedPointValue(value);
}

void
IntonationWindow::on_slopeLineEdit_editingFinished()
{
	bool ok;
	double slope = ui_->slopeLineEdit->text().toDouble(&ok);
	if (!ok) {
		ui_->slopeLineEdit->clear();
		return;
	}
	ui_->intonationWidget->setSelectedPointSlope(slope);
}

void
IntonationWindow::on_beatOffsetLineEdit_editingFinished()
{
	bool ok;
	double beatOffset = ui_->beatOffsetLineEdit->text().toDouble(&ok);
	if (!ok) {
		ui_->beatOffsetLineEdit->clear();
		return;
	}
	ui_->intonationWidget->setSelectedPointBeatOffset(beatOffset);
}

// Slot.
void
IntonationWindow::setPointData(double value, double slope, double beat, double beatOffset, double absoluteTime)
{
	ui_->valueLineEdit->setText(QString::number(value));
	ui_->slopeLineEdit->setText(QString::number(slope));
	ui_->beatLineEdit->setText(QString::number(beat));
	ui_->beatOffsetLineEdit->setText(QString::number(beatOffset));
	ui_->absoluteTimeLineEdit->setText(QString::number(absoluteTime));
}

} // namespace GS
