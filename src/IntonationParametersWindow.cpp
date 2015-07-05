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

#include "IntonationParametersWindow.h"

#include "Controller.h"
#include "Synthesis.h"
#include "TRMConfiguration.h"
#include "ui_IntonationParametersWindow.h"



namespace {

int
changeIntonation(int oldIntonation, GS::TRMControlModel::Configuration::Intonation field, Qt::CheckState state)
{
	if (state == Qt::Checked) {
		return oldIntonation | static_cast<int>(field);
	} else {
		return oldIntonation & ~static_cast<int>(field);
	}
}

} // namespace

namespace GS {

IntonationParametersWindow::IntonationParametersWindow(QWidget *parent)
		: QWidget(parent)
		, ui_(new Ui::IntonationParametersWindow)
{
	ui_->setupUi(this);

	// Intonation parameters.

	ui_->notionalPitchSpinBox->setRange(-20.0, 20.0);
	ui_->notionalPitchSpinBox->setSingleStep(0.1);
	ui_->notionalPitchSpinBox->setDecimals(1);

	ui_->pretonicRangeSpinBox->setRange(-20.0, 20.0);
	ui_->pretonicRangeSpinBox->setSingleStep(0.1);
	ui_->pretonicRangeSpinBox->setDecimals(1);

	ui_->pretonicLiftSpinBox->setRange(-20.0, 20.0);
	ui_->pretonicLiftSpinBox->setSingleStep(0.1);
	ui_->pretonicLiftSpinBox->setDecimals(1);

	ui_->tonicRangeSpinBox->setRange(-20.0, 20.0);
	ui_->tonicRangeSpinBox->setSingleStep(0.1);
	ui_->tonicRangeSpinBox->setDecimals(1);

	ui_->tonicMovementSpinBox->setRange(-20.0, 20.0);
	ui_->tonicMovementSpinBox->setSingleStep(0.1);
	ui_->tonicMovementSpinBox->setDecimals(1);

	// Drift parameters.

	ui_->deviationSpinBox->setRange(0.0, 10.0);
	ui_->deviationSpinBox->setSingleStep(0.1);
	ui_->deviationSpinBox->setDecimals(1);

	ui_->cutoffSpinBox->setRange(0.1, 10.0);
	ui_->cutoffSpinBox->setSingleStep(0.1);
	ui_->cutoffSpinBox->setDecimals(1);

	adjustSize();
}

IntonationParametersWindow::~IntonationParametersWindow()
{
}

void
IntonationParametersWindow::clear()
{
	ui_->notionalPitchSpinBox->setValue(0.0);
	ui_->pretonicRangeSpinBox->setValue(0.0);
	ui_->pretonicLiftSpinBox->setValue(0.0);
	ui_->tonicRangeSpinBox->setValue(0.0);
	ui_->tonicMovementSpinBox->setValue(0.0);

	ui_->deviationSpinBox->setValue(0.0);
	ui_->cutoffSpinBox->setValue(0.1);

	ui_->microCheckBox->setCheckState(Qt::Unchecked);
	ui_->macroCheckBox->setCheckState(Qt::Unchecked);
	ui_->smoothCheckBox->setCheckState(Qt::Unchecked);
	ui_->randomCheckBox->setCheckState(Qt::Unchecked);
	ui_->driftCheckBox->setCheckState(Qt::Unchecked);
}

void
IntonationParametersWindow::setup(Synthesis* synthesis)
{
	if (synthesis == nullptr) {
		clear();
		return;
	}

	synthesis_ = synthesis;

	TRMControlModel::Configuration& config = synthesis_->trmController->trmControlModelConfiguration();

	ui_->deviationSpinBox->setValue(config.driftDeviation);
	ui_->cutoffSpinBox->setValue(config.driftLowpassCutoff);

	ui_->microCheckBox->setCheckState( (config.intonation & TRMControlModel::Configuration::INTONATION_MICRO    ) ? Qt::Checked : Qt::Unchecked);
	ui_->macroCheckBox->setCheckState( (config.intonation & TRMControlModel::Configuration::INTONATION_MACRO    ) ? Qt::Checked : Qt::Unchecked);
	ui_->smoothCheckBox->setCheckState((config.intonation & TRMControlModel::Configuration::INTONATION_SMOOTH   ) ? Qt::Checked : Qt::Unchecked);
	ui_->randomCheckBox->setCheckState((config.intonation & TRMControlModel::Configuration::INTONATION_RANDOMIZE) ? Qt::Checked : Qt::Unchecked);
	ui_->driftCheckBox->setCheckState( (config.intonation & TRMControlModel::Configuration::INTONATION_DRIFT    ) ? Qt::Checked : Qt::Unchecked);

	ui_->notionalPitchSpinBox->setValue(config.notionalPitch);
	ui_->pretonicRangeSpinBox->setValue(config.pretonicRange);
	ui_->pretonicLiftSpinBox->setValue(config.pretonicLift);
	ui_->tonicRangeSpinBox->setValue(config.tonicRange);
	ui_->tonicMovementSpinBox->setValue(config.tonicMovement);
}

void
IntonationParametersWindow::on_updateButton_clicked()
{
	if (synthesis_ == nullptr) return;

	TRMControlModel::Configuration& config = synthesis_->trmController->trmControlModelConfiguration();

	config.intonation = changeIntonation(config.intonation, TRMControlModel::Configuration::INTONATION_MICRO    , ui_->microCheckBox->checkState());
	config.intonation = changeIntonation(config.intonation, TRMControlModel::Configuration::INTONATION_MACRO    , ui_->macroCheckBox->checkState());
	config.intonation = changeIntonation(config.intonation, TRMControlModel::Configuration::INTONATION_SMOOTH   , ui_->smoothCheckBox->checkState());
	config.intonation = changeIntonation(config.intonation, TRMControlModel::Configuration::INTONATION_RANDOMIZE, ui_->randomCheckBox->checkState());
	config.intonation = changeIntonation(config.intonation, TRMControlModel::Configuration::INTONATION_DRIFT    , ui_->driftCheckBox->checkState());

	config.driftDeviation = ui_->deviationSpinBox->value();
	config.driftLowpassCutoff = ui_->cutoffSpinBox->value();

	auto& eventList = synthesis_->trmController->eventList();
	if (ui_->useParametersCheckBox->checkState() == Qt::Checked) {
		eventList.setFixedIntonationParameters(
					ui_->notionalPitchSpinBox->value(),
					ui_->pretonicRangeSpinBox->value(),
					ui_->pretonicLiftSpinBox->value(),
					ui_->tonicRangeSpinBox->value(),
					ui_->tonicMovementSpinBox->value());
		eventList.setUseFixedIntonationParameters(true);
	} else {
		eventList.setUseFixedIntonationParameters(false);
	}
}

} // namespace GS
