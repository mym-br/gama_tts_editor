/***************************************************************************
 *  Copyright 2014, 2015 Marcelo Y. Matuda                                 *
 *  Copyright 1991, 1992, 1993, 1994, 1995, 1996, 2001, 2002               *
 *    David R. Hill, Leonard Manzara, Craig Schock                         *
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
// 2015-01
// This file was created by Marcelo Y. Matuda, and code/information
// from Gnuspeech was added to it later.

#include "TransitionEditorWindow.h"

#include <utility> /* swap */

#include <QDoubleValidator>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QTreeWidgetItem>

#include "Exception.h"
#include "Model.h"
#include "ui_TransitionEditorWindow.h"

#define NUM_EQUATIONS_TREE_COLUMNS 1
#define NUM_POINTS_TABLE_COLUMNS 6
#define NUM_POINTS_TABLE_COLUMNS_SPECIAL 4
#define PARAMETERS_MIN 1.0
#define PARAMETERS_MAX 1000.0
#define PARAMETERS_DECIMALS 1



namespace {

const char* typeNames[] = {
	"Invalid",
	"Invalid",
	"Diphone",
	"Triphone",
	"Tetraphone"
};

} // namespace

namespace GS {

TransitionEditorWindow::TransitionEditorWindow(QWidget* parent)
		: QWidget(parent)
		, ui_{std::make_unique<Ui::TransitionEditorWindow>()}
		, special_{false}
		, model_{}
		, transition_{}
		, transitionType_{VTMControlModel::Transition::TYPE_INVALID}
{
	ui_->setupUi(this);

	ui_->transitionTypeComboBox->addItem(typeNames[2], 2);
	ui_->transitionTypeComboBox->addItem(typeNames[3], 3);
	ui_->transitionTypeComboBox->addItem(typeNames[4], 4);

	ui_->equationsTree->setColumnCount(NUM_EQUATIONS_TREE_COLUMNS);
	ui_->equationsTree->setHeaderLabels(QStringList() << tr("Equation"));

	QFontMetrics fm = fontMetrics();
	int rowHeight = fm.height() + fm.ascent();

	QHeaderView* vHeader = ui_->pointsTable->verticalHeader();
	vHeader->setSectionResizeMode(QHeaderView::Fixed);
	vHeader->setDefaultSectionSize(rowHeight);
	ui_->pointsTable->setColumnCount(NUM_POINTS_TABLE_COLUMNS);
	ui_->pointsTable->setHorizontalHeaderLabels(QStringList() << tr("Type") << tr("Value") << tr("Is phantom?") << tr("Has slope?") << tr("Slope") << tr("Time"));
	//ui_->pointsTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

//	QDoubleValidator* validator = new QDoubleValidator(0.0, 1000.0, 1, this);
//	validator->setNotation(QDoubleValidator::StandardNotation);
//	ui_->ruleDurationLineEdit->setValidator(validator);
//	ui_->beatLineEdit->setValidator(validator);
//	ui_->mark1LineEdit->setValidator(validator);
//	ui_->mark2LineEdit->setValidator(validator);
//	ui_->mark3LineEdit->setValidator(validator);

	ui_->ruleDurationSpinBox->setRange(PARAMETERS_MIN, PARAMETERS_MAX);
	ui_->ruleDurationSpinBox->setDecimals(PARAMETERS_DECIMALS);

	ui_->beatSpinBox->setRange(PARAMETERS_MIN, PARAMETERS_MAX);
	ui_->beatSpinBox->setDecimals(PARAMETERS_DECIMALS);

	ui_->mark1SpinBox->setRange(PARAMETERS_MIN, PARAMETERS_MAX);
	ui_->mark1SpinBox->setDecimals(PARAMETERS_DECIMALS);

	ui_->mark2SpinBox->setRange(PARAMETERS_MIN, PARAMETERS_MAX);
	ui_->mark2SpinBox->setDecimals(PARAMETERS_DECIMALS);

	ui_->mark3SpinBox->setRange(PARAMETERS_MIN, PARAMETERS_MAX);
	ui_->mark3SpinBox->setDecimals(PARAMETERS_DECIMALS);

	connect(ui_->transitionWidget, &TransitionWidget::pointCreationRequested, this, &TransitionEditorWindow::createPoint);
}

TransitionEditorWindow::~TransitionEditorWindow()
{
}

void
TransitionEditorWindow::setSpecial()
{
	special_ = true;

	ui_->transitionWidget->setSpecial();

	setWindowTitle(tr("Special transition editor"));

	ui_->pointsTable->setColumnCount(NUM_POINTS_TABLE_COLUMNS_SPECIAL);
	ui_->pointsTable->setHorizontalHeaderLabels(QStringList() << tr("Type") << tr("Value") << tr("Is phantom?") << tr("Time"));
}

// Slot.
void
TransitionEditorWindow::clear()
{
	ui_->pointsTable->setRowCount(0);
	ui_->transitionWidget->clear();
	pointList_.clear();
	ui_->formulaTextEdit->clear();
	transitionType_ = VTMControlModel::Transition::TYPE_INVALID;
	transition_ = nullptr;

	ui_->transitionTypeComboBox->setCurrentIndex(0);
	ui_->transitionNameLabel->setText("___");
}

void
TransitionEditorWindow::resetModel(VTMControlModel::Model* model)
{
	if (model == nullptr) {
		clear();
		model_ = nullptr;
	} else {
		model_ = model;
		updateEquationsTree();
	}
}

// Slot.
void
TransitionEditorWindow::handleEditTransitionButtonClicked(unsigned int transitionGroupIndex, unsigned int transitionIndex)
{
	if (model_ == nullptr) return;

	// Find the transition instance.
	transition_ = nullptr;
	if (special_) {
		if (transitionGroupIndex < model_->specialTransitionGroupList().size()) {
			auto& transitionGroup = model_->specialTransitionGroupList()[transitionGroupIndex];
			if (transitionIndex < transitionGroup.transitionList.size()) {
				transition_ = transitionGroup.transitionList[transitionIndex].get();
			}
		}
	} else {
		if (transitionGroupIndex < model_->transitionGroupList().size()) {
			auto& transitionGroup = model_->transitionGroupList()[transitionGroupIndex];
			if (transitionIndex < transitionGroup.transitionList.size()) {
				transition_ = transitionGroup.transitionList[transitionIndex].get();
			}
		}
	}
	if (transition_ == nullptr) return;
	transitionType_ = transition_->type();

	// Set transition name.
	ui_->transitionNameLabel->setText(transition_->name().c_str());

	// Set transition type in the combo box.
	int typeIndex = ui_->transitionTypeComboBox->findData(transitionType_);
	if (typeIndex == -1) {
		qCritical("Invalid transition type: %d.", transitionType_);
		clear();
		return;
	}
	ui_->transitionTypeComboBox->setCurrentIndex(typeIndex);

	fillDefaultParameters();

	TransitionPoint::copyPointsFromTransition(*transition_, pointList_);

	try {
		updatePointTimes();
	} catch (const Exception& exc) {
		QMessageBox::critical(this, tr("Error"), exc.what());
		//TODO: clear?
		return;
	}

	if (special_) {
		TransitionPoint::sortPointListByTypeAndTime(pointList_);
	} else {
		TransitionPoint::adjustValuesInSlopeRatios(pointList_);
	}

	updatePointsTable();
	ui_->pointsTable->resizeColumnsToContents();

	updateTransitionWidget();

	ui_->equationsTree->setCurrentItem(nullptr);
	ui_->pointsTable->setCurrentItem(nullptr);

	show();
	raise();
}

// Slot.
void
TransitionEditorWindow::updateTransition()
{
	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;

	try {
		updatePointTimes();
	} catch (const Exception& exc) {
		QMessageBox::critical(this, tr("Error"), exc.what());
		//TODO: clear?
		return;
	}

	if (!special_) {
		TransitionPoint::adjustValuesInSlopeRatios(pointList_);
	}

	updatePointsTable();
	updateTransitionWidget();
}

void
TransitionEditorWindow::on_equationsTree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
	qDebug("on_equationsTree_currentItemChanged");

	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;
	if (current == nullptr) {
		ui_->formulaTextEdit->clear();
		return;
	}
	QTreeWidgetItem* parent = current->parent();
	if (parent == nullptr) { // root item
		ui_->formulaTextEdit->clear();
		return;
	}
	int parentIndex = ui_->equationsTree->indexOfTopLevelItem(parent);
	int index = parent->indexOfChild(current);
	const auto& equation = model_->equationGroupList()[parentIndex].equationList[index];

	ui_->formulaTextEdit->setPlainText(equation->formula().c_str());
}

void
TransitionEditorWindow::on_equationsTree_itemClicked(QTreeWidgetItem* item, int /*column*/)
{
	qDebug("on_equationsTree_itemClicked");

	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;
	if (item == nullptr) return;

	QTreeWidgetItem* parent = item->parent();
	if (parent == nullptr) { // root item
		ui_->formulaTextEdit->clear();
		return;
	}
	int parentIndex = ui_->equationsTree->indexOfTopLevelItem(parent);
	int index = parent->indexOfChild(item);
	const auto& equation = model_->equationGroupList()[parentIndex].equationList[index];

	// Set timeExpression for the current point.

	QTableWidgetItem* currentItem = ui_->pointsTable->currentItem();
	if (currentItem == nullptr) return;

	int row = currentItem->row();

	if (pointList_[row].timeExpression.lock() != equation) {
		pointList_[row].timeExpression = equation;
		updateTransition();
	}
}

void
TransitionEditorWindow::on_pointsTable_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* /*previous*/)
{
	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;
	if (current == nullptr) return;

	int row = ui_->pointsTable->row(current);

	qDebug("on_pointsTable_currentItemChanged row=%d", row);

	ui_->transitionWidget->setSelectedPointIndex(row);

	const std::shared_ptr<VTMControlModel::Equation> timeExpression(pointList_[row].timeExpression.lock());
	if (timeExpression) {
		unsigned int groupIndex;
		unsigned int equationIndex;
		if (model_->findEquationIndex(timeExpression->name(), groupIndex, equationIndex)) {
			QTreeWidgetItem* topLevelItem = ui_->equationsTree->topLevelItem(groupIndex);
			if (topLevelItem == nullptr) {
				qCritical("[TransitionEditorWindow::on_pointsTable_currentItemChanged]: Invalid group index: %u.", groupIndex);
				return;
			}
			QTreeWidgetItem* item = topLevelItem->child(equationIndex);
			if (item == nullptr) {
				qCritical("[TransitionEditorWindow::on_pointsTable_currentItemChanged]: Invalid index: %u for groupIndex: %u.", equationIndex, groupIndex);
				return;
			}
			ui_->equationsTree->setCurrentItem(item);
		}
	} else {
		ui_->equationsTree->setCurrentItem(nullptr);
	}
}

void
TransitionEditorWindow::on_pointsTable_itemChanged(QTableWidgetItem* item)
{
	qDebug("on_pointsTable_itemChanged");

	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;

	int row = item->row();
	int col = item->column();

	bool updateWidgets = false;
	switch (col) {
	case 1: // Value
		pointList_[row].value = item->data(Qt::DisplayRole).toFloat();
		updateWidgets = true;
		break;
	case 2: // Is phantom?
		pointList_[row].isPhantom = (item->checkState() == Qt::Checked);
		break;
	case 3: // Has slope?
		if (special_) return;
		pointList_[row].hasSlope = (item->checkState() == Qt::Checked);
		updateWidgets = true;
		break;
	case 4: // Slope
		if (special_) return;
		pointList_[row].slope = item->data(Qt::DisplayRole).toFloat();
		updateWidgets = true;
		break;
	default:
		return;
	}

	if (updateWidgets) {
		updateTransition();
	}
}

void
TransitionEditorWindow::on_updateParametersButton_clicked()
{
	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;

	updateTransition();
}

void
TransitionEditorWindow::on_removePointButton_clicked()
{
	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;

	QTableWidgetItem* currentItem = ui_->pointsTable->currentItem();
	if (currentItem == nullptr) return;

	int row = currentItem->row();
	pointList_.erase(pointList_.begin() + row);

	if (!special_) {
		TransitionPoint::adjustValuesInSlopeRatios(pointList_);
	}

	updatePointsTable();
	updateTransitionWidget();

	ui_->pointsTable->setCurrentItem(nullptr);
	ui_->equationsTree->setCurrentItem(nullptr);
}

void
TransitionEditorWindow::on_movePointUpButton_clicked()
{
	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;

	QTableWidgetItem* currentItem = ui_->pointsTable->currentItem();
	if (currentItem == nullptr) return;

	int row = currentItem->row();
	if (row == 0) return;

	if (pointList_[row].type == pointList_[row - 1].type) {
		std::swap(pointList_[row], pointList_[row - 1]);

		if (!special_) {
			TransitionPoint::adjustValuesInSlopeRatios(pointList_);
		}

		updatePointsTable();
		updateTransitionWidget();

		ui_->pointsTable->setCurrentCell(row - 1, 0);
	}
}

void
TransitionEditorWindow::on_movePointDownButton_clicked()
{
	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;

	QTableWidgetItem* currentItem = ui_->pointsTable->currentItem();
	if (currentItem == nullptr) return;

	int row = currentItem->row();
	if (row == static_cast<int>(pointList_.size()) - 1) return;

	if (pointList_[row].type == pointList_[row + 1].type) {
		std::swap(pointList_[row], pointList_[row + 1]);

		if (!special_) {
			TransitionPoint::adjustValuesInSlopeRatios(pointList_);
		}

		updatePointsTable();
		updateTransitionWidget();

		ui_->pointsTable->setCurrentCell(row + 1, 0);
	}
}

void
TransitionEditorWindow::on_updateTransitionButton_clicked()
{
	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;

	if (!special_) {
		TransitionPoint::adjustValuesInSlopeRatios(pointList_);
	}

	try {
		TransitionPoint::copyPointsToTransition(transitionType_, pointList_, *transition_);
	} catch (const Exception& exc) {
		QMessageBox::critical(this, tr("Error"), exc.what());
		return;
	}

	emit transitionChanged();
	emit equationReferenceChanged();

	//clear();
	//hide();
}

void
TransitionEditorWindow::on_transitionTypeComboBox_currentIndexChanged(int index)
{
	qDebug("on_transitionTypeComboBox_currentIndexChanged index=%d", index);

	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;

	if (index >= 0) {
		transitionType_ = static_cast<VTMControlModel::Transition::Type>(ui_->transitionTypeComboBox->itemData(index).toInt());

		fillDefaultParameters();

		updateTransition();
	}
}

// Slot.
void
TransitionEditorWindow::createPoint(unsigned int pointType, float time, float value)
{
	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;
	if (pointType < 2 || pointType > 4) {
		qCritical("Invalid point type: %u", pointType);
		return;
	}

	TransitionPoint newPoint;
	newPoint.type = static_cast<VTMControlModel::Transition::Point::Type>(pointType);
	newPoint.value = value;
	newPoint.freeTime = time;

	QTableWidget* table = ui_->pointsTable;
	if (table->rowCount() == 0) {
		pointList_.push_back(std::move(newPoint));
	} else {
		// Insert point at the end of the group.
		auto iter = pointList_.begin();
		auto end = pointList_.end();
		while (iter != end && static_cast<unsigned int>(iter->type) <= pointType) {
			++iter;
		}
		pointList_.insert(iter, std::move(newPoint));
	}

	updateTransition();

	ui_->pointsTable->setCurrentItem(nullptr);
	ui_->equationsTree->setCurrentItem(nullptr);
}

void
TransitionEditorWindow::closeEvent(QCloseEvent* event)
{
	clear();
	event->accept();
}

// Slot.
void
TransitionEditorWindow::updateEquationsTree()
{
	if (model_ == nullptr) return;

	QTreeWidget* tree = ui_->equationsTree;

	tree->clear();
	for (const auto& group : model_->equationGroupList()) {
		auto item = std::make_unique<QTreeWidgetItem>();
		item->setText(0, group.name.c_str());
		item->setFlags(Qt::ItemIsEnabled);

		for (const auto& equation : group.equationList) {
			auto childItem = std::make_unique<QTreeWidgetItem>();
			childItem->setText(0, equation->name().c_str());
			childItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			item->addChild(childItem.release());
		}

		tree->addTopLevelItem(item.release());
	}
	tree->expandAll();
}

void
TransitionEditorWindow::fillDefaultParameters()
{
	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;

	model_->setDefaultFormulaSymbols(transitionType_);
	ui_->ruleDurationSpinBox->setValue(model_->getFormulaSymbolValue(VTMControlModel::FormulaSymbol::SYMB_RD));
	ui_->beatSpinBox->setValue(        model_->getFormulaSymbolValue(VTMControlModel::FormulaSymbol::SYMB_BEAT));
	ui_->mark1SpinBox->setValue(       model_->getFormulaSymbolValue(VTMControlModel::FormulaSymbol::SYMB_MARK1));
	ui_->mark2SpinBox->setValue(       model_->getFormulaSymbolValue(VTMControlModel::FormulaSymbol::SYMB_MARK2));
	ui_->mark3SpinBox->setValue(       model_->getFormulaSymbolValue(VTMControlModel::FormulaSymbol::SYMB_MARK3));
}

void
TransitionEditorWindow::updatePointTimes()
{
	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;

	model_->setDefaultFormulaSymbols(transitionType_);
	model_->setFormulaSymbolValue(VTMControlModel::FormulaSymbol::SYMB_RD, ui_->ruleDurationSpinBox->value());
	model_->setFormulaSymbolValue(VTMControlModel::FormulaSymbol::SYMB_BEAT, ui_->beatSpinBox->value());
	model_->setFormulaSymbolValue(VTMControlModel::FormulaSymbol::SYMB_MARK1, ui_->mark1SpinBox->value());
	model_->setFormulaSymbolValue(VTMControlModel::FormulaSymbol::SYMB_MARK2, ui_->mark2SpinBox->value());
	model_->setFormulaSymbolValue(VTMControlModel::FormulaSymbol::SYMB_MARK3, ui_->mark3SpinBox->value());

	TransitionPoint::calculateTimes(*model_, pointList_);
}

void
TransitionEditorWindow::updatePointsTable()
{
	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;

	QTableWidget* table = ui_->pointsTable;

	QSignalBlocker blocker(table);

	table->setRowCount(pointList_.size());
	for (unsigned int i = 0, size = pointList_.size(); i < size; ++i) {
		const auto& point = pointList_[i];
		unsigned int column = 0;

		auto item = std::make_unique<QTableWidgetItem>(QString("%1 - %2").arg(point.type).arg(typeNames[point.type]));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(i, column++, item.release());

		item = std::make_unique<QTableWidgetItem>();
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
		item->setData(Qt::DisplayRole, point.value);
		table->setItem(i, column++, item.release());

		item = std::make_unique<QTableWidgetItem>();
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		item->setCheckState(point.isPhantom ? Qt::Checked : Qt::Unchecked);
		table->setItem(i, column++, item.release());

		if (!special_) {
			item = std::make_unique<QTableWidgetItem>();
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			item->setCheckState(point.hasSlope ? Qt::Checked : Qt::Unchecked);
			table->setItem(i, column++, item.release());

			item = std::make_unique<QTableWidgetItem>();
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
			item->setData(Qt::DisplayRole, point.slope);
			table->setItem(i, column++, item.release());
		}

		const std::shared_ptr<VTMControlModel::Equation> timeExpression(point.timeExpression.lock());
		item = std::make_unique<QTableWidgetItem>(timeExpression ? timeExpression->name().c_str() : "");
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(i, column, item.release());
	}
}

void
TransitionEditorWindow::updateTransitionWidget()
{
	if (model_ == nullptr) return;
	if (transition_ == nullptr) return;

	ui_->transitionWidget->updateData(
		transitionType_,
		&pointList_,
		ui_->ruleDurationSpinBox->value(),
		ui_->mark1SpinBox->value(),
		ui_->mark2SpinBox->value(),
		ui_->mark3SpinBox->value()
	);
}

} // namespace GS
