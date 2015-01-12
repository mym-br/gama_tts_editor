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

#ifndef RULE_EDITOR_WINDOW_H
#define RULE_EDITOR_WINDOW_H

#include <memory>

#include <QWidget>



namespace Ui {
class RuleEditorWindow;
}

QT_BEGIN_NAMESPACE
class QTableWidgetItem;
class QTreeWidgetItem;
QT_END_NAMESPACE

namespace GS {

namespace TRMControlModel {
class Model;
class Rule;
}

class RuleEditorWindow : public QWidget {
	Q_OBJECT
public:
	explicit RuleEditorWindow(QWidget* parent=0);
	~RuleEditorWindow();

	void clear();
	void resetModel(TRMControlModel::Model* model);
public slots:
	void handleEditRuleButtonClicked(unsigned int ruleIndex);
private slots:
	void on_clearSpecialTransitionButton_clicked();
	void on_clearEquationButton_clicked();
	void on_closeButton_clicked();
	void on_ruleTransitionsTable_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
	void on_ruleSpecialTransitionsTable_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
	void on_ruleSymbolEquationsTable_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
	void on_transitionsTree_itemClicked(QTreeWidgetItem* item, int column);
	void on_specialTransitionsTree_itemClicked(QTreeWidgetItem* item, int column);
	void on_equationsTree_itemClicked(QTreeWidgetItem* item, int column);
private:
	void setupRuleTransitionsTable();
	void setupTransitionsTree();
	void setupRuleSpecialTransitionsTable();
	void setupSpecialTransitionsTree();
	void setupRuleSymbolEquationsTable();
	void setupEquationsTree();

	std::unique_ptr<Ui::RuleEditorWindow> ui_;
	TRMControlModel::Model* model_;
	TRMControlModel::Rule* rule_;
};

} // namespace GS

#endif // RULE_EDITOR_WINDOW_H
