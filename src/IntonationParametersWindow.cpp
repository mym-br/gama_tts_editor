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
#include "ui_IntonationParametersWindow.h"



namespace {

int
changeIntonation(int oldIntonation, GS::VTMControlModel::Configuration::Intonation field, Qt::CheckState state)
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
		, ui_{std::make_unique<Ui::IntonationParametersWindow>()}
{
	ui_->setupUi(this);

	// Intonation parameters.

	ui_->notionalPitchSpinBox->setRange(-20.0, 20.0);
	ui_->notionalPitchSpinBox->setSingleStep(0.1);
	ui_->notionalPitchSpinBox->setDecimals(1);

	ui_->pretonicPitchRangeSpinBox->setRange(-20.0, 20.0);
	ui_->pretonicPitchRangeSpinBox->setSingleStep(0.1);
	ui_->pretonicPitchRangeSpinBox->setDecimals(1);

	ui_->pretonicPerturbationRangeSpinBox->setRange(-20.0, 20.0);
	ui_->pretonicPerturbationRangeSpinBox->setSingleStep(0.1);
	ui_->pretonicPerturbationRangeSpinBox->setDecimals(1);

	ui_->tonicPitchRangeSpinBox->setRange(-20.0, 20.0);
	ui_->tonicPitchRangeSpinBox->setSingleStep(0.1);
	ui_->tonicPitchRangeSpinBox->setDecimals(1);

	ui_->tonicPerturbationRangeSpinBox->setRange(-20.0, 20.0);
	ui_->tonicPerturbationRangeSpinBox->setSingleStep(0.1);
	ui_->tonicPerturbationRangeSpinBox->setDecimals(1);

	// Drift parameters.

	ui_->deviationSpinBox->setRange(0.0, 100.0);
	ui_->deviationSpinBox->setSingleStep(0.1);
	ui_->deviationSpinBox->setDecimals(1);

	ui_->cutoffSpinBox->setRange(0.1, 100.0);
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
	ui_->pretonicPitchRangeSpinBox->setValue(0.0);
	ui_->pretonicPerturbationRangeSpinBox->setValue(0.0);
	ui_->tonicPitchRangeSpinBox->setValue(0.0);
	ui_->tonicPerturbationRangeSpinBox->setValue(0.0);

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

	VTMControlModel::Configuration& config = synthesis_->vtmController->vtmControlModelConfiguration();

	ui_->deviationSpinBox->setValue(config.driftDeviation);
	ui_->cutoffSpinBox->setValue(config.driftLowpassCutoff);

	ui_->microCheckBox->setCheckState( (config.intonation & VTMControlModel::Configuration::INTONATION_MICRO ) ? Qt::Checked : Qt::Unchecked);
	ui_->macroCheckBox->setCheckState( (config.intonation & VTMControlModel::Configuration::INTONATION_MACRO ) ? Qt::Checked : Qt::Unchecked);
	ui_->smoothCheckBox->setCheckState((config.intonation & VTMControlModel::Configuration::INTONATION_SMOOTH) ? Qt::Checked : Qt::Unchecked);
	ui_->randomCheckBox->setCheckState((config.intonation & VTMControlModel::Configuration::INTONATION_RANDOM) ? Qt::Checked : Qt::Unchecked);
	ui_->driftCheckBox->setCheckState( (config.intonation & VTMControlModel::Configuration::INTONATION_DRIFT ) ? Qt::Checked : Qt::Unchecked);

	ui_->notionalPitchSpinBox->setValue(            config.notionalPitch);
	ui_->pretonicPitchRangeSpinBox->setValue(       config.pretonicPitchRange);
	ui_->pretonicPerturbationRangeSpinBox->setValue(config.pretonicPerturbationRange);
	ui_->tonicPitchRangeSpinBox->setValue(          config.tonicPitchRange);
	ui_->tonicPerturbationRangeSpinBox->setValue(   config.tonicPerturbationRange);
}

void
IntonationParametersWindow::on_updateButton_clicked()
{
	if (synthesis_ == nullptr) return;

	VTMControlModel::Configuration& config = synthesis_->vtmController->vtmControlModelConfiguration();

	config.intonation = changeIntonation(config.intonation, VTMControlModel::Configuration::INTONATION_MICRO , ui_->microCheckBox->checkState());
	config.intonation = changeIntonation(config.intonation, VTMControlModel::Configuration::INTONATION_MACRO , ui_->macroCheckBox->checkState());
	config.intonation = changeIntonation(config.intonation, VTMControlModel::Configuration::INTONATION_SMOOTH, ui_->smoothCheckBox->checkState());
	config.intonation = changeIntonation(config.intonation, VTMControlModel::Configuration::INTONATION_RANDOM, ui_->randomCheckBox->checkState());
	config.intonation = changeIntonation(config.intonation, VTMControlModel::Configuration::INTONATION_DRIFT , ui_->driftCheckBox->checkState());

	config.driftDeviation = ui_->deviationSpinBox->value();
	config.driftLowpassCutoff = ui_->cutoffSpinBox->value();

	auto& eventList = synthesis_->vtmController->eventList();
	if (ui_->useParametersCheckBox->checkState() == Qt::Checked) {
		eventList.setFixedIntonationParameters(
					ui_->notionalPitchSpinBox->value(),
					ui_->pretonicPitchRangeSpinBox->value(),
					ui_->pretonicPerturbationRangeSpinBox->value(),
					ui_->tonicPitchRangeSpinBox->value(),
					ui_->tonicPerturbationRangeSpinBox->value());
		eventList.setUseFixedIntonationParameters(true);
	} else {
		eventList.setUseFixedIntonationParameters(false);
	}
}

} // namespace GS
