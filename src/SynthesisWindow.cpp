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

#define TRM_PARAM_FILE_NAME "generated__trm_param.txt"
#define SPEECH_FILE_NAME "generated__speech.wav"



namespace GS {

SynthesisWindow::SynthesisWindow(QWidget* parent)
		: QWidget(parent)
		, ui_(new Ui::SynthesisWindow)
		, model_(nullptr)
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
	model_ = nullptr;
	phoneticStringParser_.reset(0);
	textParser_.reset(0);
	trmController_.reset(0);
	projectDir_.clear();
}

void
SynthesisWindow::setup(const QString& projectDir, TRMControlModel::Model* model)
{
	if (model == nullptr) {
		clear();
		return;
	}
	if (model->parameterList().size() != NUM_PARAM) {
		qWarning("[SynthesisWindow::setup] Wrong number of parameters: %lu (should be %d).", model->parameterList().size(), NUM_PARAM);
	}

	try {
		projectDir_ = projectDir;
		const std::string configDirPath = projectDir_.toStdString();
		trmController_.reset(new TRMControlModel::Controller(configDirPath.c_str(), *model));
		textParser_.reset(new En::TextParser(configDirPath.c_str()));
		phoneticStringParser_.reset(new En::PhoneticStringParser(configDirPath.c_str(), *trmController_));
		model_ = model;

		ui_->eventWidget->updateData(&trmController_->eventList(), model_);
		ui_->eventWidget->clearParameterSelection();

		// Fill the parameter table.
		auto* table = ui_->parameterTableWidget;
		table->setRowCount(model_->parameterList().size());
		for (unsigned int i = 0; i < model_->parameterList().size(); ++i) {
			std::unique_ptr<QTableWidgetItem> item(new QTableWidgetItem(model_->parameterList()[i].name().c_str()));
			item->setFlags(/*Qt::ItemIsSelectable | Qt::ItemIsEditable |*/ Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			item->setCheckState(Qt::Unchecked);
			table->setItem(i, 0, item.release());

			item.reset(new QTableWidgetItem);
			item->setFlags(/*Qt::ItemIsSelectable | Qt::ItemIsEditable |*/ Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			item->setCheckState(Qt::Unchecked);
			table->setItem(i, 1, item.release());
		}
		table->resizeColumnsToContents();
	} catch (...) {
		clear();
		throw;
	}
}

void
SynthesisWindow::on_parseButton_clicked()
{
	if (!trmController_) {
		return;
	}
	QString text = ui_->textLineEdit->text();
	if (text.trimmed().isEmpty()) {
		return;
	}

	try {
		std::string phoneticString = textParser_->parseText(text.toStdString().c_str());
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
	if (!trmController_) {
		return;
	}
	QString phoneticString = ui_->phoneticStringTextEdit->toPlainText();
	if (phoneticString.trimmed().isEmpty()) {
		return;
	}

	try {
		QString trmParamFilePath = projectDir_ + TRM_PARAM_FILE_NAME;
		QString speechFilePath = projectDir_ + SPEECH_FILE_NAME;

		trmController_->synthesizePhoneticString(
					*phoneticStringParser_,
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

} // namespace GS
