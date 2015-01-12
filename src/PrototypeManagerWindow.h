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

#ifndef PROTOTYPE_MANAGER_WINDOW_H
#define PROTOTYPE_MANAGER_WINDOW_H

#include <memory>
#include <vector>

#include <QWidget>

#include "TransitionPoint.h"



QT_BEGIN_NAMESPACE
class QTreeWidgetItem;
QT_END_NAMESPACE

namespace Ui {
class PrototypeManagerWindow;
}

namespace GS {

namespace TRMControlModel {
class Equation;
class Model;
class Transition;
}

class PrototypeManagerWindow : public QWidget {
	Q_OBJECT
public:
	explicit PrototypeManagerWindow(QWidget* parent=0);
	~PrototypeManagerWindow();

	void resetModel(TRMControlModel::Model* model);
signals:
	void editTransitionButtonClicked(unsigned int transitionGroupIndex, unsigned int transitionIndex);
	void editSpecialTransitionButtonClicked(unsigned int transitionGroupIndex, unsigned int transitionIndex);
private slots:
	void on_addEquationButton_clicked();
	void on_removeEquationButton_clicked();
	void on_moveEquationUpButton_clicked();
	void on_moveEquationDownButton_clicked();
	void on_updateEquationButton_clicked();
	void on_equationsTree_itemChanged(QTreeWidgetItem* item, int column);
	void on_equationsTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

	void on_addTransitionButton_clicked();
	void on_removeTransitionButton_clicked();
	void on_moveTransitionUpButton_clicked();
	void on_moveTransitionDownButton_clicked();
	void on_updateTransitionButton_clicked();
	void on_editTransitionButton_clicked();
	void on_transitionsTree_itemChanged(QTreeWidgetItem* item, int column);
	void on_transitionsTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

	void on_addSpecialTransitionButton_clicked();
	void on_removeSpecialTransitionButton_clicked();
	void on_moveSpecialTransitionUpButton_clicked();
	void on_moveSpecialTransitionDownButton_clicked();
	void on_updateSpecialTransitionButton_clicked();
	void on_editSpecialTransitionButton_clicked();
	void on_specialTransitionsTree_itemChanged(QTreeWidgetItem* item, int column);
	void on_specialTransitionsTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
private:
	void setupEquationsTree();
	void clearEquationData();

	void setupTransitionsTree();
	void clearTransitionData();

	void setupSpecialTransitionsTree();
	void clearSpecialTransitionData();

	std::unique_ptr<Ui::PrototypeManagerWindow> ui_;

	TRMControlModel::Model* model_;
	TRMControlModel::Equation* currentEquation_;
	TRMControlModel::Transition* currentTransition_;
	TRMControlModel::Transition* currentSpecialTransition_;

	std::vector<TransitionPoint> pointList_;
	std::vector<TransitionPoint> specialPointList_;
};

} // namespace GS

#endif // PROTOTYPE_MANAGER_WINDOW_H
