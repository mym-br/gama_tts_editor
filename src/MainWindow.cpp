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

#include "MainWindow.h"

#include <iostream>
#include <utility> /* move */

#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextEdit>
#include <QVBoxLayout>

#include "DataEntryWindow.h"
#include "IntonationWindow.h"
#include "IntonationParametersWindow.h"
#include "LogStreamBuffer.h"
#include "Model.h"
#include "PostureEditorWindow.h"
#include "PrototypeManagerWindow.h"
#include "RuleManagerWindow.h"
#include "RuleTesterWindow.h"
#include "Synthesis.h"
#include "SynthesisWindow.h"
#include "TransitionEditorWindow.h"
#include "ui_MainWindow.h"

#define PROGRAM_NAME "MonetCP"
#define PROGRAM_VERSION "0.1.4"
#define MAX_LOG_BLOCK_COUNT 500



namespace GS {

MainWindow::MainWindow(QWidget* parent)
		: QMainWindow(parent)
		, config_()
		, model_()
		, synthesis_(new Synthesis)
		, ui_(new Ui::MainWindow)
		, dataEntryWindow_(new DataEntryWindow)
		, intonationWindow_(new IntonationWindow)
		, intonationParametersWindow_(new IntonationParametersWindow)
		, postureEditorWindow_(new PostureEditorWindow)
		, prototypeManagerWindow_(new PrototypeManagerWindow)
		, specialTransitionEditorWindow_(new TransitionEditorWindow)
		, ruleManagerWindow_(new RuleManagerWindow)
		, ruleTesterWindow_(new RuleTesterWindow)
		, synthesisWindow_(new SynthesisWindow)
		, transitionEditorWindow_(new TransitionEditorWindow)
{
	ui_->setupUi(this);

	ui_->logTextEdit->setMaximumBlockCount(MAX_LOG_BLOCK_COUNT);
	specialTransitionEditorWindow_->setSpecial();

	connect(ui_->quitAction, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
	connect(ui_->aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	coutStreamBuffer_.reset(new LogStreamBuffer(std::cout, false, ui_->logTextEdit));
	cerrStreamBuffer_.reset(new LogStreamBuffer(std::cerr, true, ui_->logTextEdit));
	LogStreamBuffer::registerQDebugMessageHandler();

	connect(prototypeManagerWindow_.get(), SIGNAL(editTransitionButtonClicked(unsigned int, unsigned int)),
		transitionEditorWindow_.get(), SLOT(handleEditTransitionButtonClicked(unsigned int, unsigned int)));

	connect(prototypeManagerWindow_.get(), SIGNAL(editSpecialTransitionButtonClicked(unsigned int, unsigned int)),
		specialTransitionEditorWindow_.get(), SLOT(handleEditTransitionButtonClicked(unsigned int, unsigned int)));

	connect(dataEntryWindow_.get(), SIGNAL(categoryChanged()) , postureEditorWindow_.get(), SLOT(unselectPosture()));
	connect(dataEntryWindow_.get(), SIGNAL(parameterChanged()), postureEditorWindow_.get(), SLOT(unselectPosture()));
	connect(dataEntryWindow_.get(), SIGNAL(symbolChanged())   , postureEditorWindow_.get(), SLOT(unselectPosture()));

	connect(dataEntryWindow_.get(), SIGNAL(parameterChanged()), ruleManagerWindow_.get(), SLOT(loadRuleData()));
	connect(dataEntryWindow_.get(), SIGNAL(parameterChanged()), synthesisWindow_.get(), SLOT(setupParameterTable()));

	connect(postureEditorWindow_.get(), SIGNAL(postureChanged()), ruleManagerWindow_.get(), SLOT(unselectRule()));
	connect(postureEditorWindow_.get(), SIGNAL(postureCategoryChanged()), dataEntryWindow_.get(), SLOT(updateCategoriesTable()));

	connect(prototypeManagerWindow_.get(), SIGNAL(equationChanged()), transitionEditorWindow_.get(), SLOT(updateEquationsTree()));
	connect(prototypeManagerWindow_.get(), SIGNAL(equationChanged()), transitionEditorWindow_.get(), SLOT(updateTransition()));
	connect(prototypeManagerWindow_.get(), SIGNAL(equationChanged()), specialTransitionEditorWindow_.get(), SLOT(updateEquationsTree()));
	connect(prototypeManagerWindow_.get(), SIGNAL(equationChanged()), specialTransitionEditorWindow_.get(), SLOT(updateTransition()));
	connect(prototypeManagerWindow_.get(), SIGNAL(equationChanged()), ruleManagerWindow_.get(), SLOT(setupRuleSymbolEquationsTable()));
	connect(prototypeManagerWindow_.get(), SIGNAL(equationChanged()), ruleManagerWindow_.get(), SLOT(setupEquationsTree()));

	connect(prototypeManagerWindow_.get(), SIGNAL(transitionChanged()), transitionEditorWindow_.get(), SLOT(clear()));
	connect(prototypeManagerWindow_.get(), SIGNAL(transitionChanged()), ruleManagerWindow_.get(), SLOT(setupRuleTransitionsTable()));
	connect(prototypeManagerWindow_.get(), SIGNAL(transitionChanged()), ruleManagerWindow_.get(), SLOT(setupTransitionsTree()));

	connect(prototypeManagerWindow_.get(), SIGNAL(specialTransitionChanged()), specialTransitionEditorWindow_.get(), SLOT(clear()));
	connect(prototypeManagerWindow_.get(), SIGNAL(specialTransitionChanged()), ruleManagerWindow_.get(), SLOT(setupRuleSpecialTransitionsTable()));
	connect(prototypeManagerWindow_.get(), SIGNAL(specialTransitionChanged()), ruleManagerWindow_.get(), SLOT(setupSpecialTransitionsTree()));

	connect(transitionEditorWindow_.get(), SIGNAL(equationReferenceChanged()), prototypeManagerWindow_.get(), SLOT(setupEquationsTree()));
	connect(transitionEditorWindow_.get(), SIGNAL(transitionChanged()),        prototypeManagerWindow_.get(), SLOT(unselectTransition()));

	connect(specialTransitionEditorWindow_.get(), SIGNAL(equationReferenceChanged()), prototypeManagerWindow_.get(), SLOT(setupEquationsTree()));
	connect(specialTransitionEditorWindow_.get(), SIGNAL(transitionChanged()),        prototypeManagerWindow_.get(), SLOT(unselectSpecialTransition()));

	connect(ruleManagerWindow_.get(), SIGNAL(categoryReferenceChanged()), dataEntryWindow_.get(), SLOT(updateCategoriesTable()));

	connect(ruleManagerWindow_.get(), SIGNAL(transitionReferenceChanged())       , prototypeManagerWindow_.get(), SLOT(setupTransitionsTree()));
	connect(ruleManagerWindow_.get(), SIGNAL(specialTransitionReferenceChanged()), prototypeManagerWindow_.get(), SLOT(setupSpecialTransitionsTree()));
	connect(ruleManagerWindow_.get(), SIGNAL(equationReferenceChanged())         , prototypeManagerWindow_.get(), SLOT(setupEquationsTree()));

	connect(synthesisWindow_.get(), SIGNAL(textSynthesized()), intonationWindow_.get(), SLOT(loadIntonationFromEventList()));
	connect(synthesisWindow_.get(), SIGNAL(audioStarted()), intonationWindow_.get(), SLOT(handleAudioStarted()));
	connect(synthesisWindow_.get(), SIGNAL(audioFinished()), intonationWindow_.get(), SLOT(handleAudioFinished()));
	connect(synthesisWindow_.get(), SIGNAL(synthesisFinished()), intonationWindow_.get(), SLOT(handleSynthesisFinished()));
	connect(intonationWindow_.get(), SIGNAL(synthesisRequested()), synthesisWindow_.get(), SLOT(synthesizeWithManualIntonation()));
	connect(intonationWindow_.get(), SIGNAL(synthesisToFileRequested(QString)), synthesisWindow_.get(), SLOT(synthesizeToFileWithManualIntonation(QString)));
}

MainWindow::~MainWindow()
{
}

void
MainWindow::closeEvent(QCloseEvent* event)
{
	QMessageBox::StandardButton ret;
	ret = QMessageBox::warning(this, tr("Quit"), tr("Do you want to quit the application?"),
					QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (ret == QMessageBox::Yes) {
		event->accept();
		qApp->closeAllWindows();
	} else {
		event->ignore();
	}
}

void
MainWindow::on_openAction_triggered()
{
	QString dir = config_.projectDir;
	if (dir.isNull()) {
		dir = QDir::currentPath();
	}
	QString filePath = QFileDialog::getOpenFileName(this, tr("Open file"), dir, tr("XML files (*.xml)"));
	if (filePath.isEmpty()) {
		return;
	}

	QFileInfo fileInfo(filePath);
	config_.projectDir = fileInfo.absolutePath() + '/';
	config_.origConfigFileName = fileInfo.fileName();
	config_.newConfigFileName = QString();

	openModel();
}

void
MainWindow::on_saveAction_triggered()
{
	if (model_.get() == nullptr || config_.origConfigFileName.isNull()) {
		return;
	}

	if (config_.newConfigFileName.isNull()) {
		while (!selectNewConfigFileName()) {}
	}

	if (!config_.newConfigFileName.isEmpty()) {
		saveModel();
	}
}

void
MainWindow::on_saveAsAction_triggered()
{
	if (model_.get() == nullptr || config_.origConfigFileName.isNull()) {
		return;
	}

	while (!selectNewConfigFileName()) {}

	if (!config_.newConfigFileName.isEmpty()) {
		saveModel();
	}
}

void
MainWindow::on_revertAction_triggered()
{
	if (config_.projectDir.isEmpty() || config_.origConfigFileName.isEmpty()) {
		return;
	}

	openModel();
}

void
MainWindow::on_dataEntryButton_clicked()
{
	dataEntryWindow_->show();
	dataEntryWindow_->raise();
}

void
MainWindow::on_ruleManagerButton_clicked()
{
	ruleManagerWindow_->show();
	ruleManagerWindow_->raise();
}

void
MainWindow::on_prototypeManagerButton_clicked()
{
	prototypeManagerWindow_->show();
	prototypeManagerWindow_->raise();
}

void
MainWindow::on_postureEditorButton_clicked()
{
	postureEditorWindow_->show();
	postureEditorWindow_->raise();
}

void
MainWindow::on_intonationWindowButton_clicked()
{
	intonationWindow_->show();
	intonationWindow_->raise();
}

void
MainWindow::on_ruleTesterButton_clicked()
{
	ruleTesterWindow_->show();
	ruleTesterWindow_->raise();
}

void
MainWindow::on_synthesisWindowButton_clicked()
{
	synthesisWindow_->show();
	synthesisWindow_->raise();
}

void
MainWindow::on_intonationParametersButton_clicked()
{
	intonationParametersWindow_->show();
	intonationParametersWindow_->raise();
}

bool
MainWindow::selectNewConfigFileName()
{
	QString filePath = QFileDialog::getSaveFileName(this, tr("Save file"), config_.projectDir + "new_" + config_.origConfigFileName, tr("XML files (*.xml)"));
	if (!filePath.isEmpty()) {
		QFileInfo fileInfo(filePath);
		QString dir = fileInfo.absolutePath() + '/';
		if (dir != config_.projectDir) {
			QMessageBox::critical(this, tr("Error"), tr("The directory must be the same of the original file."));
			return false;
		}
		config_.newConfigFileName = fileInfo.fileName();
	}
	return true;
}

void
MainWindow::openModel()
{
	try {
		model_.reset(new TRMControlModel::Model);
		model_->load(config_.projectDir.toStdString().c_str(), config_.origConfigFileName.toStdString().c_str());

		synthesis_->setup(config_.projectDir, model_.get());

		dataEntryWindow_->resetModel(model_.get());
		postureEditorWindow_->resetModel(model_.get());
		prototypeManagerWindow_->resetModel(model_.get());
		transitionEditorWindow_->resetModel(model_.get());
		specialTransitionEditorWindow_->resetModel(model_.get());
		ruleManagerWindow_->resetModel(model_.get());
		ruleTesterWindow_->resetModel(model_.get());
		synthesisWindow_->setup(model_.get(), synthesis_.get());
		intonationWindow_->setup(synthesis_.get());
		intonationParametersWindow_->setup(synthesis_.get());

		qDebug("### Model opened.");
	} catch (const Exception& exc) {
		QMessageBox::critical(this, tr("Error"), exc.what());

		intonationParametersWindow_->setup(nullptr);
		intonationWindow_->setup(nullptr);
		synthesisWindow_->setup(nullptr, nullptr);
		ruleTesterWindow_->resetModel(nullptr);
		ruleManagerWindow_->resetModel(nullptr);
		specialTransitionEditorWindow_->resetModel(nullptr);
		transitionEditorWindow_->resetModel(nullptr);
		prototypeManagerWindow_->resetModel(nullptr);
		postureEditorWindow_->resetModel(nullptr);
		dataEntryWindow_->resetModel(nullptr);

		synthesis_->setup(QString(), nullptr);

		model_.reset();
	}
}

void
MainWindow::saveModel()
{
	try {
		model_->save(config_.projectDir.toStdString().c_str(), config_.newConfigFileName.toStdString().c_str());

		qDebug("### Model saved.");
	} catch (const Exception& exc) {
		QMessageBox::critical(this, tr("Error"), exc.what());
	}
}

// Slot.
void
MainWindow::about()
{
	QDialog aboutDialog(this);
	QVBoxLayout* layout = new QVBoxLayout(&aboutDialog);

	QTextEdit* textEdit = new QTextEdit(&aboutDialog);
	textEdit->setReadOnly(true);
	textEdit->setHtml(
		"<pre>"
		PROGRAM_NAME " " PROGRAM_VERSION "\n\n"

		"This program is free software: you can redistribute it and/or modify\n"
		"it under the terms of the GNU General Public License as published by\n"
		"the Free Software Foundation, either version 3 of the License, or\n"
		"(at your option) any later version.\n\n"

		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
		"GNU General Public License for more details.\n\n"

		"You should have received a copy of the GNU General Public License\n"
		"along with this program. If not, see http://www.gnu.org/licenses/.\n"
		"</pre>"

		"<hr/>"

		"<pre>"
		"This program uses code from:\n\n"

		"- The Gnuspeech project (http://www.gnu.org/software/gnuspeech/).\n\n"

		"  Provided by David R. Hill, Leonard Manzara and Craig Schock.\n\n"

		"  Gnuspeech is distributed under the terms of the GNU General Public License\n"
		"  as published by the Free Software Foundation, either version 3 of the\n"
		"  License, or (at your option) any later version.\n\n"

		"- Qt (http://www.qt.io/).\n\n"

		"  Provided by Digia Plc.\n\n"

		"  Qt Free Edition is distributed under the terms of the GNU LGPLv2.1 or LGPLv3,\n"
		"  depending on the library (see http://www.qt.io/faq/).\n\n"

		"- PortAudio Portable Real-Time Audio Library (http://www.portaudio.com/).\n\n"

		"  Provided by Ross Bencina and Phil Burk.\n\n"

		"  PortAudio is distributed under these terms:\n"
		"  \"Permission is hereby granted, free of charge, to any person obtaining a copy\n"
		"  of this software and associated documentation files (the \"Software\"), to\n"
		"  deal in the Software without restriction, including without limitation the\n"
		"  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or\n"
		"  sell copies of the Software, and to permit persons to whom the Software is\n"
		"  furnished to do so, subject to the following conditions:\n\n"

		"  The above copyright notice and this permission notice shall be included in\n"
		"  all copies or substantial portions of the Software.\n\n"

		"  THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
		"  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
		"  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n"
		"  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,\n"
		"  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR\n"
		"  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE\n"
		"  USE OR OTHER DEALINGS IN THE SOFTWARE.\"\n\n"

		"</pre>"
	);
	layout->addWidget(textEdit);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, &aboutDialog);
	layout->addWidget(buttonBox);

	connect(buttonBox, SIGNAL(accepted()), &aboutDialog, SLOT(accept()));

	aboutDialog.setWindowTitle(tr("About ") + PROGRAM_NAME);
	aboutDialog.resize(600, 550);
	aboutDialog.exec();
}

} // namespace GS
