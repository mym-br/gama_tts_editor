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

#include "ParameterModificationWindow.h"

#include <exception>

#include <QMessageBox>

#include "Controller.h"
#include "Model.h"
#include "ParameterModificationSynthesis.h"
#include "Synthesis.h"
#include "ui_ParameterModificationWindow.h"

#define DEFAULT_AMPLITUDE (10.0)
#define ADD_AMPLITUDE_INCREMENT (0.1)
#define MIN_AMPLITUDE_SPINBOX_VALUE (0.0)
#define MAX_AMPLITUDE_SPINBOX_VALUE (60.0)
#define DEFAULT_OUTPUT_GAIN (0.5)
#define GAIN_INCREMENT (0.01)



namespace GS {

ParameterModificationWindow::ParameterModificationWindow(QWidget* parent)
		: QWidget{parent}
		, ui_{std::make_unique<Ui::ParameterModificationWindow>()}
		, model_{}
		, synthesis_{}
		, prevAmplitude_{DEFAULT_AMPLITUDE}
		, state_{State::stopped}
		, modificationValue_{}
		, modificationTimer_{this}
{
	ui_->setupUi(this);

	ui_->amplitudeSpinBox->setSingleStep(ADD_AMPLITUDE_INCREMENT);
	ui_->amplitudeSpinBox->setMinimum(MIN_AMPLITUDE_SPINBOX_VALUE);
	ui_->amplitudeSpinBox->setMaximum(MAX_AMPLITUDE_SPINBOX_VALUE);
	ui_->amplitudeSpinBox->setValue(DEFAULT_AMPLITUDE);

	ui_->outputGainSpinBox->setSingleStep(GAIN_INCREMENT);
	ui_->outputGainSpinBox->setMinimum(0.0);
	ui_->outputGainSpinBox->setMaximum(1.0);
	ui_->outputGainSpinBox->setValue(DEFAULT_OUTPUT_GAIN);

	ui_->parameterCurveWidget->addGraph(); // original parameters
	ui_->parameterCurveWidget->graph(0)->setPen(QPen(Qt::black));
	ui_->parameterCurveWidget->graph(0)->setAntialiased(true);

	ui_->parameterCurveWidget->addGraph(); // modified parameters
	ui_->parameterCurveWidget->graph(1)->setPen(QPen(Qt::blue));
	ui_->parameterCurveWidget->graph(1)->setAntialiased(true);

	connect(ui_->parameterModificationWidget, &ParameterModificationWidget::modificationStarted,
			this, &ParameterModificationWindow::handleModificationStarted);
	connect(ui_->parameterModificationWidget, &ParameterModificationWidget::offsetChanged,
			this, &ParameterModificationWindow::handleOffsetChanged);

	modificationTimer_.setTimerType(Qt::PreciseTimer);
	connect(&modificationTimer_, &QTimer::timeout,
			this, &ParameterModificationWindow::sendModificationValue);
}

ParameterModificationWindow::~ParameterModificationWindow()
{
}

void
ParameterModificationWindow::clear()
{
	ui_->parameterComboBox->clear();
	synthesis_ = nullptr;
	model_ = nullptr;
}

void
ParameterModificationWindow::setup(VTMControlModel::Model* model, Synthesis* synthesis)
{
	if (!model || !synthesis || !synthesis->vtmController) {
		clear();
		return;
	}

	model_ = model;
	synthesis_ = synthesis;

	// Fill parameters combobox.
	ui_->parameterComboBox->clear();
	for (unsigned int i = 0, size = model_->parameterList().size(); i < size; ++i) {
		ui_->parameterComboBox->addItem(model_->parameterList()[i].name().c_str(), i);
	}

	try {
		synthesis_->paramModifSynth.reset();
		synthesis_->paramModifSynth = std::make_unique<ParameterModificationSynthesis>(
			model_->parameterList().size(),
			synthesis_->vtmController->vtmControlModelConfiguration().controlRate,
			synthesis_->vtmController->vtmConfigData());
	} catch (...) {
		clear();
		throw;
	}
}

void
ParameterModificationWindow::resetData()
{
	if (!model_) return;

	synthesis_->paramModifSynth->processor().resetData(synthesis_->vtmController->vtmParameterList());

	// Fill the x-axis in the parameter graph.
	modifParamX_.resize(synthesis_->vtmController->vtmParameterList().size());
	const double period = 1.0 / synthesis_->vtmController->vtmControlModelConfiguration().controlRate;
	for (std::size_t i = 0, size = modifParamX_.size(); i < size; ++i) {
		modifParamX_[i] = i * period * 1000.0; // convert to milliseconds
	}
}

void
ParameterModificationWindow::on_resetParameterButton_clicked()
{
	if (!model_) return;
}

void
ParameterModificationWindow::on_synthesizeButton_clicked()
{
	if (!model_) return;
}

void
ParameterModificationWindow::on_synthesizeToFileButton_clicked()
{
	if (!model_) return;
}

void
ParameterModificationWindow::on_parameterComboBox_currentIndexChanged(int /*index*/)
{
	if (!model_) return;
}

void
ParameterModificationWindow::on_addRadioButton_toggled(bool checked)
{
	if (checked) {
		ui_->amplitudeSpinBox->setMinimum(MIN_AMPLITUDE_SPINBOX_VALUE);
		ui_->amplitudeSpinBox->setMaximum(MAX_AMPLITUDE_SPINBOX_VALUE);
		ui_->amplitudeSpinBox->setValue(prevAmplitude_);
	} else {
		prevAmplitude_ = ui_->amplitudeSpinBox->value();
		ui_->amplitudeSpinBox->setMinimum(1.0);
		ui_->amplitudeSpinBox->setMaximum(1.0);
		ui_->amplitudeSpinBox->setValue(1.0);
	}
}

// Slot.
void
ParameterModificationWindow::handleModificationStarted()
{
	if (!model_) return;

	if (state_ == State::stopped) {
		try {
			synthesis_->paramModifSynth->startSynthesis(
				synthesis_->vtmController->outputScale() * ui_->outputGainSpinBox->value());
		} catch (const std::exception& exc) {
			QMessageBox::critical(this, tr("Error"), exc.what());
			return;
		}
		modificationTimer_.start(MODIF_TIMER_INTERVAL_MS);
		state_ = State::running;
	}
}

// Slot.
void
ParameterModificationWindow::handleOffsetChanged(double offset)
{
	if (!model_) return;

	if (state_ == State::stopped) return;

	double modificationValue;
	if (ui_->addRadioButton->isChecked()) {
		modificationValue = offset * ui_->amplitudeSpinBox->value();
	} else {
		modificationValue = 1.0 + offset;
		if (modificationValue < 0.0) {
			modificationValue = 0.0;
		}
	}

	modificationValue_ = modificationValue;
}

// Slot.
void
ParameterModificationWindow::sendModificationValue()
{
	if (!model_) return;

	if (!synthesis_->paramModifSynth->modifyParameter(
				ui_->parameterComboBox->currentIndex(),
				ui_->addRadioButton->isChecked() ? 0 : 1,
				modificationValue_)) {
		state_ = State::stopped;
		modificationTimer_.stop();

		showModifiedParameterData();
	}
}

void
ParameterModificationWindow::showModifiedParameterData()
{
	if (!model_) {
		ui_->parameterCurveWidget->graph(0)->clearData();
		ui_->parameterCurveWidget->graph(1)->clearData();
		return;
	}

	synthesis_->paramModifSynth->processor().getParameter(ui_->parameterComboBox->currentIndex(), paramY_);
	synthesis_->paramModifSynth->processor().getModifiedParameter(ui_->parameterComboBox->currentIndex(), modifParamY_);

	ui_->parameterCurveWidget->graph(0)->setData(modifParamX_, paramY_);
	ui_->parameterCurveWidget->graph(0)->rescaleAxes();
	ui_->parameterCurveWidget->graph(1)->setData(modifParamX_, modifParamY_);
	ui_->parameterCurveWidget->graph(1)->rescaleAxes(true);
	ui_->parameterCurveWidget->replot();
}

} // namespace GS
