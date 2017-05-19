/***************************************************************************
 *  Copyright 2014, 2015 Marcelo Y. Matuda                                 *
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

#include "SynthesisWindow.h"

#include <memory>
#include <string>

#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QStringList>

#include "AudioWorker.h"
#include "Controller.h"
#include "PhoneticStringParser.h"
#include "en/text_parser/TextParser.h"
#include "Model.h"
#include "Synthesis.h"
#include "ui_SynthesisWindow.h"

#define VTM_PARAM_FILE_NAME "generated__vtm_param.txt"



namespace GS {

SynthesisWindow::SynthesisWindow(QWidget* parent)
		: QWidget {parent}
		, ui_ {std::make_unique<Ui::SynthesisWindow>()}
		, model_ {}
		, synthesis_ {}
		, audioWorker_ {}
{
	ui_->setupUi(this);

	QFontMetrics fm = fontMetrics();
	int rowHeight = fm.height() + fm.ascent();

	QHeaderView* vHeader = ui_->parameterTableWidget->verticalHeader();
	vHeader->setSectionResizeMode(QHeaderView::Fixed);
	vHeader->setDefaultSectionSize(rowHeight);
	ui_->parameterTableWidget->setColumnCount(2);
	ui_->parameterTableWidget->setHorizontalHeaderLabels(QStringList() << tr("Parameter") << tr("Special"));

	ui_->tempoSpinBox->setRange(0.1, 10.0);
	ui_->tempoSpinBox->setSingleStep(0.1);
	ui_->tempoSpinBox->setDecimals(1);
	ui_->tempoSpinBox->setValue(1.0);

	connect(ui_->textLineEdit, &QLineEdit::returnPressed, ui_->parseButton, &QPushButton::click);
	connect(ui_->eventWidget , &EventWidget::mouseMoved , this            , &SynthesisWindow::updateMouseTracking);

	audioWorker_ = new AudioWorker;
	audioWorker_->moveToThread(&audioThread_);
	connect(&audioThread_, &QThread::finished,
			audioWorker_, &AudioWorker::deleteLater);
	connect(this         , &SynthesisWindow::playAudioRequested,
			audioWorker_, &AudioWorker::playAudio);
	connect(audioWorker_ , &AudioWorker::finished,
			this        , &SynthesisWindow::handleAudioFinished);
	connect(audioWorker_ , &AudioWorker::errorOccurred,
			this        , &SynthesisWindow::handleAudioError);
	connect(this         , &SynthesisWindow::updateAudioDeviceComboBoxRequested,
			audioWorker_, &AudioWorker::sendOutputDeviceList);
	connect(audioWorker_ , &AudioWorker::audioOutputDeviceListSent,
			this        , &SynthesisWindow::updateAudioDeviceComboBox);
	audioThread_.start();
}

SynthesisWindow::~SynthesisWindow()
{
	audioThread_.quit();
	audioThread_.wait();
}

void
SynthesisWindow::clear()
{
	ui_->parameterTableWidget->setRowCount(0);
	ui_->eventWidget->updateData(nullptr, nullptr);
	synthesis_ = nullptr;
	model_ = nullptr;
}

void
SynthesisWindow::setup(VTMControlModel::Model* model, Synthesis* synthesis)
{
	if (model == nullptr || synthesis == nullptr) {
		clear();
		return;
	}
	if (model->parameterList().size() != NUM_PARAM) {
		qWarning("[SynthesisWindow::setup] Wrong number of parameters: %lu (should be %d).", model->parameterList().size(), NUM_PARAM);
		clear();
		return;
	}

	try {
		model_ = model;
		synthesis_ = synthesis;

		ui_->eventWidget->updateData(&synthesis_->vtmController->eventList(), model_);
		ui_->eventWidget->clearParameterSelection();

		setupParameterTable();
	} catch (...) {
		clear();
		throw;
	}

	emit updateAudioDeviceComboBoxRequested();
}

void
SynthesisWindow::on_parseButton_clicked()
{
	if (!synthesis_) {
		return;
	}
	QString text = ui_->textLineEdit->text();
	if (text.trimmed().isEmpty()) {
		return;
	}
	disableProcessingButtons();

	try {
		std::string phoneticString = synthesis_->textParser->parse(text.toUtf8().constData());
		ui_->phoneticStringTextEdit->setPlainText(phoneticString.c_str());
	} catch (const Exception& exc) {
		QMessageBox::critical(this, tr("Error"), exc.what());
		enableProcessingButtons();
		return;
	}

	on_synthesizeButton_clicked();
}

void
SynthesisWindow::on_synthesizeButton_clicked()
{
	if (!synthesis_) {
		return;
	}
	QString phoneticString = ui_->phoneticStringTextEdit->toPlainText();
	if (phoneticString.trimmed().isEmpty()) {
		return;
	}
	disableProcessingButtons();

	try {
		QString vtmParamFilePath = synthesis_->projectDir + VTM_PARAM_FILE_NAME;

		VTMControlModel::Configuration& config = synthesis_->vtmController->vtmControlModelConfiguration();
		config.tempo = ui_->tempoSpinBox->value();

		const ConfigurationData& vtmConfig = synthesis_->vtmController->vtmConfigurationData();
		const double sampleRate = vtmConfig.value<double>("output_rate");

		{
			std::lock_guard<std::mutex> lock(AudioPlayer::bufferMutex);

			synthesis_->vtmController->synthesizePhoneticString(
						phoneticString.toStdString().c_str(),
						vtmParamFilePath.toStdString().c_str(),
						audioWorker_->player().buffer());
		}

		ui_->eventWidget->update();

		int audioDeviceIndex = ui_->audioDeviceComboBox->currentIndex();
		if (audioDeviceIndex == -1) {
			audioDeviceIndex = 0;
		}
		emit audioStarted();
		emit playAudioRequested(sampleRate, audioDeviceIndex);

		emit textSynthesized();
	} catch (const Exception& exc) {
		QMessageBox::critical(this, tr("Error"), exc.what());
		enableProcessingButtons();
	}
}

void
SynthesisWindow::on_synthesizeToFileButton_clicked()
{
	if (!synthesis_) {
		return;
	}
	QString phoneticString = ui_->phoneticStringTextEdit->toPlainText();
	if (phoneticString.trimmed().isEmpty()) {
		return;
	}
	QString filePath = QFileDialog::getSaveFileName(this, tr("Save file"), synthesis_->projectDir, tr("WAV files (*.wav)"));
	if (filePath.isEmpty()) {
		return;
	}
	disableProcessingButtons();

	try {
		QString vtmParamFilePath = synthesis_->projectDir + VTM_PARAM_FILE_NAME;

		VTMControlModel::Configuration& config = synthesis_->vtmController->vtmControlModelConfiguration();
		config.tempo = ui_->tempoSpinBox->value();

		synthesis_->vtmController->synthesizePhoneticString(
					phoneticString.toStdString().c_str(),
					vtmParamFilePath.toStdString().c_str(),
					filePath.toStdString().c_str());

		ui_->eventWidget->update();

		emit textSynthesized();
	} catch (const Exception& exc) {
		QMessageBox::critical(this, tr("Error"), exc.what());
	}

	enableProcessingButtons();
}

// Slot.
void
SynthesisWindow::synthesizeWithManualIntonation()
{
	if (!synthesis_) {
		return;
	}
	disableProcessingButtons();

	try {
		auto& eventList = synthesis_->vtmController->eventList();
		if (eventList.list().empty()) {
			return;
		}
		eventList.clearMacroIntonation();
		eventList.prepareMacroIntonationInterpolation();

		const ConfigurationData& vtmConfig = synthesis_->vtmController->vtmConfigurationData();
		const double sampleRate = vtmConfig.value<double>("output_rate");

		QString vtmParamFilePath = synthesis_->projectDir + VTM_PARAM_FILE_NAME;

		{
			std::lock_guard<std::mutex> lock(AudioPlayer::bufferMutex);

			synthesis_->vtmController->synthesizeFromEventList(
						vtmParamFilePath.toStdString().c_str(),
						audioWorker_->player().buffer());
		}

		int audioDeviceIndex = ui_->audioDeviceComboBox->currentIndex();
		if (audioDeviceIndex == -1) {
			audioDeviceIndex = 0;
		}
		emit audioStarted();
		emit playAudioRequested(sampleRate, audioDeviceIndex);
	} catch (const Exception& exc) {
		QMessageBox::critical(this, tr("Error"), exc.what());
		enableProcessingButtons();
	}
}

// Slot.
void
SynthesisWindow::synthesizeToFileWithManualIntonation(QString filePath)
{
	if (!synthesis_) {
		return;
	}
	disableProcessingButtons();

	try {
		auto& eventList = synthesis_->vtmController->eventList();
		if (eventList.list().empty()) {
			return;
		}
		eventList.clearMacroIntonation();
		eventList.prepareMacroIntonationInterpolation();

		QString vtmParamFilePath = synthesis_->projectDir + VTM_PARAM_FILE_NAME;

		synthesis_->vtmController->synthesizeFromEventList(
					vtmParamFilePath.toStdString().c_str(),
					filePath.toStdString().c_str());

	} catch (const Exception& exc) {
		QMessageBox::critical(this, tr("Error"), exc.what());
	}

	enableProcessingButtons();
}

void
SynthesisWindow::on_parameterTableWidget_cellChanged(int row, int column)
{
	bool selected = ui_->parameterTableWidget->item(row, column)->checkState() == Qt::Checked;
	ui_->eventWidget->changeParameterSelection(row, column == 1, selected);
}

// Slot.
void
SynthesisWindow::setupParameterTable()
{
	if (model_ == nullptr) return;

	auto* table = ui_->parameterTableWidget;
	table->setRowCount(model_->parameterList().size());
	for (unsigned int i = 0; i < model_->parameterList().size(); ++i) {
		auto item = std::make_unique<QTableWidgetItem>(model_->parameterList()[i].name().c_str());
		item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		item->setCheckState(Qt::Unchecked);
		table->setItem(i, 0, item.release());

		item = std::make_unique<QTableWidgetItem>();
		item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		item->setCheckState(Qt::Unchecked);
		table->setItem(i, 1, item.release());
	}
	table->resizeColumnsToContents();
}

// Slot.
void
SynthesisWindow::updateMouseTracking(double time, double value)
{
	if (time < -0.1) {
		ui_->timeLineEdit->clear();
		ui_->valueLineEdit->clear();
	} else {
		ui_->timeLineEdit->setText(QString::number(time, 'f', 0));
		ui_->valueLineEdit->setText(QString::number(value, 'f', 2));
	}
}

// Slot.
void
SynthesisWindow::handleAudioError(QString msg)
{
	QMessageBox::critical(this, tr("Error"), msg);

	enableProcessingButtons();

	emit updateAudioDeviceComboBoxRequested();
	emit audioFinished();
}

// Slot.
void
SynthesisWindow::handleAudioFinished()
{
	enableProcessingButtons();

	emit updateAudioDeviceComboBoxRequested();
	emit audioFinished();
}

// Slot.
void
SynthesisWindow::updateAudioDeviceComboBox(QStringList deviceNameList, int defaultDeviceIndex)
{
	int oldIndex = ui_->audioDeviceComboBox->currentIndex();
	int oldCount = ui_->audioDeviceComboBox->count();

	ui_->audioDeviceComboBox->clear();

	if (deviceNameList.empty()) {
		ui_->audioDeviceComboBox->setCurrentIndex(-1);
		return;
	}

	ui_->audioDeviceComboBox->addItems(deviceNameList);

	if (oldIndex == -1 || oldCount != ui_->audioDeviceComboBox->count()) {
		ui_->audioDeviceComboBox->setCurrentIndex(defaultDeviceIndex);
	} else {
		ui_->audioDeviceComboBox->setCurrentIndex(oldIndex);
	}
}

void
SynthesisWindow::enableProcessingButtons()
{
	ui_->parseButton->setEnabled(true);
	ui_->synthesizeButton->setEnabled(true);
	ui_->synthesizeToFileButton->setEnabled(true);
}

void
SynthesisWindow::disableProcessingButtons()
{
	ui_->parseButton->setEnabled(false);
	ui_->synthesizeButton->setEnabled(false);
	ui_->synthesizeToFileButton->setEnabled(false);
}

} // namespace GS
