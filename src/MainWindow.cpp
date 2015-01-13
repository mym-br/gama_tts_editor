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
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include "DataEntryWindow.h"
#include "LogStreamBuffer.h"
#include "Model.h"
#include "PostureEditorWindow.h"
#include "PrototypeManagerWindow.h"
#include "RuleEditorWindow.h"
#include "RuleManagerWindow.h"
#include "RuleTesterWindow.h"
#include "SynthesisWindow.h"
#include "TransitionEditorWindow.h"
#include "ui_MainWindow.h"

#define MAX_LOG_BLOCK_COUNT 500



namespace GS {

MainWindow::MainWindow(QWidget* parent)
		: QMainWindow(parent)
		, config_()
		, model_()
		, ui_(new Ui::MainWindow)
		, dataEntryWindow_(new DataEntryWindow)
		, postureEditorWindow_(new PostureEditorWindow)
		, prototypeManagerWindow_(new PrototypeManagerWindow)
		, specialTransitionEditorWindow_(new TransitionEditorWindow)
		, ruleEditorWindow_(new RuleEditorWindow)
		, ruleManagerWindow_(new RuleManagerWindow)
		, ruleTesterWindow_(new RuleTesterWindow)
		, synthesisWindow_(new SynthesisWindow)
		, transitionEditorWindow_(new TransitionEditorWindow)
{
	ui_->setupUi(this);

	ui_->logTextEdit->setMaximumBlockCount(MAX_LOG_BLOCK_COUNT);
	specialTransitionEditorWindow_->setSpecial();

	connect(ui_->quitAction, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

	coutStreamBuffer_.reset(new LogStreamBuffer(std::cout, ui_->logTextEdit));
	cerrStreamBuffer_.reset(new LogStreamBuffer(std::cerr, ui_->logTextEdit));
	LogStreamBuffer::registerQDebugMessageHandler();

	connect(prototypeManagerWindow_.get(), SIGNAL(editTransitionButtonClicked(unsigned int, unsigned int)),
		transitionEditorWindow_.get(), SLOT(handleEditTransitionButtonClicked(unsigned int, unsigned int)));

	connect(prototypeManagerWindow_.get(), SIGNAL(editSpecialTransitionButtonClicked(unsigned int, unsigned int)),
		specialTransitionEditorWindow_.get(), SLOT(handleEditTransitionButtonClicked(unsigned int, unsigned int)));

	connect(ruleManagerWindow_.get(), SIGNAL(editRuleButtonClicked(unsigned int)),
		ruleEditorWindow_.get(), SLOT(handleEditRuleButtonClicked(unsigned int)));




	connect(dataEntryWindow_.get(), SIGNAL(categoryChanged()) , postureEditorWindow_.get(), SLOT(unselectPosture()));
	connect(dataEntryWindow_.get(), SIGNAL(parameterChanged()), postureEditorWindow_.get(), SLOT(unselectPosture()));
	connect(dataEntryWindow_.get(), SIGNAL(symbolChanged())   , postureEditorWindow_.get(), SLOT(unselectPosture()));

	connect(dataEntryWindow_.get(), SIGNAL(parameterChanged()), ruleEditorWindow_.get(), SLOT(clearRuleData()));


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
	//QString filePath = QFileDialog::getOpenFileName(this, tr("Open file"), dir, tr("XML files (*.xml)"));
	QString filePath = "../gs_editor/data/monet.xml"; //TODO: remove test
	if (filePath.isEmpty()) {
		return;
	}

	QFileInfo fileInfo(filePath);
	config_.projectDir = fileInfo.absolutePath() + '/';
	config_.origConfigFileName = fileInfo.fileName();
	config_.newConfigFileName = QString();

	try {
		std::unique_ptr<TRMControlModel::Model> newModel(new TRMControlModel::Model);
		newModel->load(config_.projectDir.toStdString().c_str(), config_.origConfigFileName.toStdString().c_str());
		dataEntryWindow_->resetModel(newModel.get());
		postureEditorWindow_->resetModel(newModel.get());
		prototypeManagerWindow_->resetModel(newModel.get());
		transitionEditorWindow_->resetModel(newModel.get());
		specialTransitionEditorWindow_->resetModel(newModel.get());
		ruleEditorWindow_->resetModel(newModel.get());
		ruleManagerWindow_->resetModel(newModel.get());
		ruleTesterWindow_->resetModel(newModel.get());
		synthesisWindow_->setup(config_.projectDir, newModel.get());

		model_ = std::move(newModel); // must be executed after all the previous resetModel() calls
	} catch (const Exception& exc) {
		QMessageBox::critical(this, tr("Error"), exc.what());
		synthesisWindow_->setup(QString(), nullptr);
		ruleTesterWindow_->resetModel(nullptr);
		ruleManagerWindow_->resetModel(nullptr);
		ruleEditorWindow_->resetModel(nullptr);
		specialTransitionEditorWindow_->resetModel(nullptr);
		transitionEditorWindow_->resetModel(nullptr);
		prototypeManagerWindow_->resetModel(nullptr);
		postureEditorWindow_->resetModel(nullptr);
		dataEntryWindow_->resetModel(nullptr);
		model_.reset();
	}
}

void
MainWindow::on_saveAction_triggered()
{
	if (model_.get() == nullptr || config_.origConfigFileName.isNull()) {
		return;
	}

	if (config_.newConfigFileName.isNull()) {
		while (true) {
			QString filePath = QFileDialog::getSaveFileName(this, tr("Save file"), config_.projectDir + "new_" + config_.origConfigFileName, tr("XML files (*.xml)"));
			if (!filePath.isEmpty()) {
				QFileInfo fileInfo(filePath);
				QString dir = fileInfo.absolutePath() + '/';
				if (dir != config_.projectDir) {
					QMessageBox::critical(this, tr("Error"), tr("The directory must be the same of the original file."));
					continue;
				}
				config_.newConfigFileName = fileInfo.fileName();
			}
			break;
		}
	}

	if (!config_.newConfigFileName.isEmpty()) {
		try {
			model_->save(config_.projectDir.toStdString().c_str(), config_.newConfigFileName.toStdString().c_str());
		} catch (const Exception& exc) {
			QMessageBox::critical(this, tr("Error"), exc.what());
		}
	}
}

void
MainWindow::on_dataEntryAction_triggered()
{
	dataEntryWindow_->show();
	dataEntryWindow_->raise();
}

void
MainWindow::on_postureEditorAction_triggered()
{
	postureEditorWindow_->show();
	postureEditorWindow_->raise();
}

void
MainWindow::on_prototypeManagerAction_triggered()
{
	prototypeManagerWindow_->show();
	prototypeManagerWindow_->raise();
}

void
MainWindow::on_transitionEditorAction_triggered()
{
	transitionEditorWindow_->show();
	transitionEditorWindow_->raise();
}

void
MainWindow::on_specialTransitionEditorAction_triggered()
{
	specialTransitionEditorWindow_->show();
	specialTransitionEditorWindow_->raise();
}

void
MainWindow::on_ruleEditorAction_triggered()
{
	ruleEditorWindow_->show();
	ruleEditorWindow_->raise();
}

void
MainWindow::on_ruleManagerAction_triggered()
{
	ruleManagerWindow_->show();
	ruleManagerWindow_->raise();
}

void
MainWindow::on_ruleTesterAction_triggered()
{
	ruleTesterWindow_->show();
	ruleTesterWindow_->raise();
}

void
MainWindow::on_synthesisWindowAction_triggered()
{
	synthesisWindow_->show();
	synthesisWindow_->raise();
}

void
MainWindow::on_synthesizerControlPanelAction_triggered()
{

}

void
MainWindow::on_intonationWindowAction_triggered()
{

}

void
MainWindow::on_intonationParametersAction_triggered()
{

}

} // namespace GS
