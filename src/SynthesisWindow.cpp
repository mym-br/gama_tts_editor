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
#include "ui_SynthesisWindow.h"

#include <memory>
#include <string>

#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QStringList>

#include "Controller.h"
#include "en/phonetic_string_parser/PhoneticStringParser.h"
#include "en/text_parser/TextParser.h"
#include "Model.h"
#include "Synthesis.h"

#define TRM_PARAM_FILE_NAME "generated__trm_param.txt"
#define SPEECH_FILE_NAME "generated__speech.wav"



namespace GS {

SynthesisWindow::SynthesisWindow(QWidget* parent)
		: QWidget(parent)
		, ui_(new Ui::SynthesisWindow)
		, model_(nullptr)
		, synthesis_(nullptr)
{
	ui_->setupUi(this);

	QFontMetrics fm = fontMetrics();
	int rowHeight = fm.height() + fm.xHeight();

	QHeaderView* vHeader = ui_->parameterTableWidget->verticalHeader();
	vHeader->setResizeMode(QHeaderView::Fixed);
	vHeader->setDefaultSectionSize(rowHeight);
	ui_->parameterTableWidget->setColumnCount(2);
	ui_->parameterTableWidget->setHorizontalHeaderLabels(QStringList() << tr("Parameter") << tr("Special"));
}

SynthesisWindow::~SynthesisWindow()
{
}

void
SynthesisWindow::clear()
{
	synthesis_ = nullptr;
	model_ = nullptr;
}

void
SynthesisWindow::setup(TRMControlModel::Model* model, Synthesis* synthesis)
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

		ui_->eventWidget->updateData(&synthesis_->trmController->eventList(), model_);
		ui_->eventWidget->clearParameterSelection();

		setupParameterTable();
	} catch (...) {
		clear();
		throw;
	}
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

	try {
		std::string phoneticString = synthesis_->textParser->parseText(text.toStdString().c_str());
		ui_->phoneticStringTextEdit->setPlainText(phoneticString.c_str());
	} catch (const Exception& exc) {
		QMessageBox::critical(this, tr("Error"), exc.what());
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

	try {
		QString trmParamFilePath = synthesis_->projectDir + TRM_PARAM_FILE_NAME;
		QString speechFilePath = synthesis_->projectDir + SPEECH_FILE_NAME;

		synthesis_->trmController->synthesizePhoneticString(
					*synthesis_->phoneticStringParser,
					phoneticString.toStdString().c_str(),
					trmParamFilePath.toStdString().c_str(),
					speechFilePath.toStdString().c_str());

		ui_->eventWidget->update();

		//TODO: replace temporary solution
		QStringList arguments;
		arguments << speechFilePath;
		QProcess* process = new QProcess(this);
		process->start("aplay", arguments);
		// Do not check the return value.

		emit textSynthesized();
	} catch (const Exception& exc) {
		QMessageBox::critical(this, tr("Error"), exc.what());
	}
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
		std::unique_ptr<QTableWidgetItem> item(new QTableWidgetItem(model_->parameterList()[i].name().c_str()));
		item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		item->setCheckState(Qt::Unchecked);
		table->setItem(i, 0, item.release());

		item.reset(new QTableWidgetItem);
		item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		item->setCheckState(Qt::Unchecked);
		table->setItem(i, 1, item.release());
	}
	table->resizeColumnsToContents();
}

} // namespace GS
