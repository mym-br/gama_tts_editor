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

#ifndef POSTURE_EDITOR_WINDOW_H
#define POSTURE_EDITOR_WINDOW_H

#include <memory>

#include <QWidget>



namespace Ui {
class PostureEditorWindow;
}

QT_FORWARD_DECLARE_CLASS(QTableWidgetItem)

namespace GS {

namespace VTMControlModel {
class Model;
class Posture;
}

class PostureEditorWindow : public QWidget {
	Q_OBJECT
public:
	explicit PostureEditorWindow(QWidget* parent=0);
	~PostureEditorWindow();

	void resetModel(VTMControlModel::Model* model);
signals:
	void postureChanged();
	void postureCategoryChanged();
public slots:
	void unselectPosture();
private slots:
	void on_addPostureButton_clicked();
	void on_removePostureButton_clicked();
	void on_updatePostureCommentButton_clicked();
	void on_posturesTable_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem*);
	void on_posturesTable_itemChanged(QTableWidgetItem* item);
	void on_categoriesTable_itemChanged(QTableWidgetItem* item);
	void on_parametersTable_itemChanged(QTableWidgetItem* item);
	void on_useDefaultParameterValueButton_clicked();
	void on_symbolsTable_itemChanged(QTableWidgetItem* item);
	void on_useDefaultSymbolValueButton_clicked();
private:
	void setupPosturesTable();
	void clearPostureData();
	void setupCategoriesTable(const VTMControlModel::Posture& posture);
	void setupParametersTable(const VTMControlModel::Posture& posture);
	void setupSymbolsTable(const VTMControlModel::Posture& posture);

	std::unique_ptr<Ui::PostureEditorWindow> ui_;
	VTMControlModel::Model* model_;
};

} // namespace GS

#endif // POSTURE_EDITOR_WINDOW_H
