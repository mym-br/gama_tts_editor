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

#ifndef RULE_MANAGER_WINDOW_H
#define RULE_MANAGER_WINDOW_H

#include <memory>

#include <QWidget>



namespace Ui {
class RuleManagerWindow;
}

QT_BEGIN_NAMESPACE
class QString;
class QTableWidgetItem;
QT_END_NAMESPACE

namespace GS {

namespace TRMControlModel {
class Model;
class Rule;
}

class RuleManagerWindow : public QWidget {
	Q_OBJECT
public:
	explicit RuleManagerWindow(QWidget* parent=0);
	~RuleManagerWindow();

	void resetModel(TRMControlModel::Model* model);
signals:
	void editRuleButtonClicked(unsigned int ruleIndex);
private slots:
	void on_removeButton_clicked();
	void on_addButton_clicked();
	void on_updateButton_clicked();
	void on_moveUpButton_clicked();
	void on_moveDownButton_clicked();
	void on_editButton_clicked();
	void on_rulesTable_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
private:
	void clearRuleData();
	void setupRulesList();
	void showRuleStatistics(const TRMControlModel::Rule& rule);
	unsigned int numInputExpressions(const QString& exp1, const QString& exp2, const QString& exp3, const QString& exp4);

	std::unique_ptr<Ui::RuleManagerWindow> ui_;

	TRMControlModel::Model* model_;
};

} // namespace GS

#endif // RULE_MANAGER_WINDOW_H
