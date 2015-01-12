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

#ifndef TRANSITION_EDITOR_WINDOW_H
#define TRANSITION_EDITOR_WINDOW_H

#include <memory>

#include <QWidget>

#include "TransitionPoint.h"



QT_BEGIN_NAMESPACE
class QTableWidgetItem;
class QTreeWidgetItem;
QT_END_NAMESPACE

namespace Ui {
class TransitionEditorWindow;
}

namespace GS {

namespace TRMControlModel {
class Model;
class Transition;
}

class TransitionEditorWindow : public QWidget {
	Q_OBJECT
public:
	explicit TransitionEditorWindow(QWidget* parent=0);
	~TransitionEditorWindow();

	void setSpecial();
	void clear();
	void resetModel(TRMControlModel::Model* model);
public slots:
	void handleEditTransitionButtonClicked(unsigned int transitionGroupIndex, unsigned int transitionIndex);
private slots:
	void on_equationsTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void on_equationsTree_itemClicked(QTreeWidgetItem* item, int column);
	void on_pointsTable_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
	void on_pointsTable_itemChanged(QTableWidgetItem* item);
	void on_updateParametersButton_clicked();
	void on_removePointButton_clicked();
	void on_movePointUpButton_clicked();
	void on_movePointDownButton_clicked();
	void on_updateTransitionButton_clicked();
	void on_transitionTypeComboBox_currentIndexChanged(int index);
	void createPoint(unsigned int pointType, float time, float value);
private:
	void updateEquationsTree();
	void fillDefaultParameters();
	void updatePointTimes();
	void updatePointsTable();
	void updateTransitionWidget();

	std::unique_ptr<Ui::TransitionEditorWindow> ui_;
	bool special_;
	TRMControlModel::Model* model_;
	TRMControlModel::Transition* transition_;
	TRMControlModel::Transition::Type transitionType_;
	std::vector<TransitionPoint> pointList_;
};

} // namespace GS

#endif // TRANSITION_EDITOR_WINDOW_H
