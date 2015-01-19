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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <memory>

#include <QMainWindow>

#include "AppConfig.h"

namespace Ui {
class MainWindow;
}

namespace GS {

class Synthesis;

class DataEntryWindow;
class IntonationWindow;
class IntonationParametersWindow;
class LogStreamBuffer;
class PostureEditorWindow;
class PrototypeManagerWindow;
class RuleEditorWindow;
class RuleManagerWindow;
class RuleTesterWindow;
class SynthesisWindow;
class TransitionEditorWindow;

namespace TRMControlModel {
class Model;
}

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit MainWindow(QWidget* parent=0);
	~MainWindow();
protected:
	virtual void closeEvent(QCloseEvent* event);
public slots:
	void on_openAction_triggered();
	void on_saveAction_triggered();

	void on_dataEntryAction_triggered();
	void on_postureEditorAction_triggered();
	void on_prototypeManagerAction_triggered();
	void on_transitionEditorAction_triggered();
	void on_specialTransitionEditorAction_triggered();
	void on_ruleEditorAction_triggered();
	void on_ruleManagerAction_triggered();
	void on_ruleTesterAction_triggered();
	void on_synthesisWindowAction_triggered();
	void on_synthesizerControlPanelAction_triggered();
	void on_intonationWindowAction_triggered();
	void on_intonationParametersAction_triggered();
private:
	AppConfig config_;
	std::unique_ptr<TRMControlModel::Model> model_;
	std::unique_ptr<Synthesis> synthesis_;

	std::unique_ptr<Ui::MainWindow> ui_;
	std::unique_ptr<DataEntryWindow> dataEntryWindow_;
	std::unique_ptr<IntonationWindow> intonationWindow_;
	std::unique_ptr<IntonationParametersWindow> intonationParametersWindow_;
	std::unique_ptr<PostureEditorWindow> postureEditorWindow_;
	std::unique_ptr<PrototypeManagerWindow> prototypeManagerWindow_;
	std::unique_ptr<TransitionEditorWindow> specialTransitionEditorWindow_;
	std::unique_ptr<RuleEditorWindow> ruleEditorWindow_;
	std::unique_ptr<RuleManagerWindow> ruleManagerWindow_;
	std::unique_ptr<RuleTesterWindow> ruleTesterWindow_;
	std::unique_ptr<SynthesisWindow> synthesisWindow_;
	std::unique_ptr<TransitionEditorWindow> transitionEditorWindow_;
	std::unique_ptr<LogStreamBuffer> coutStreamBuffer_;
	std::unique_ptr<LogStreamBuffer> cerrStreamBuffer_;
};

} // namespace GS

#endif // MAIN_WINDOW_H
