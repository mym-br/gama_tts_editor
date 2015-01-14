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

#include "RuleEditorWindow.h"

#include <QString>

#include "Model.h"
#include "SignalBlocker.h"
#include "ui_RuleEditorWindow.h"

#define NUM_RULE_TRANSITIONS_TABLE_COLUMNS 2
#define NUM_RULE_SPECIAL_TRANSITIONS_TABLE_COLUMNS 2
#define NUM_RULE_SYMBOL_EQUATIONS_TABLE_COLUMNS 2
#define NUM_TRANSITIONS_TREE_COLUMNS 1
#define NUM_SPECIAL_TRANSITIONS_TREE_COLUMNS 1
#define NUM_EQUATIONS_TREE_COLUMNS 1



namespace GS {

RuleEditorWindow::RuleEditorWindow(QWidget *parent)
		: QWidget(parent)
		, ui_(new Ui::RuleEditorWindow)
		, model_(nullptr)
		, rule_(nullptr)
{
	ui_->setupUi(this);

	QFontMetrics fm = fontMetrics();
	int rowHeight = fm.height() + fm.xHeight();

	QHeaderView* vHeader = ui_->ruleTransitionsTable->verticalHeader();
	vHeader->setResizeMode(QHeaderView::Fixed);
	vHeader->setDefaultSectionSize(rowHeight);
	ui_->ruleTransitionsTable->setColumnCount(NUM_RULE_TRANSITIONS_TABLE_COLUMNS);
	ui_->ruleTransitionsTable->setHorizontalHeaderLabels(QStringList() << tr("Parameter") << tr("Rule transition"));
	ui_->ruleTransitionsTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

	vHeader = ui_->ruleSpecialTransitionsTable->verticalHeader();
	vHeader->setResizeMode(QHeaderView::Fixed);
	vHeader->setDefaultSectionSize(rowHeight);
	ui_->ruleSpecialTransitionsTable->setColumnCount(NUM_RULE_SPECIAL_TRANSITIONS_TABLE_COLUMNS);
	ui_->ruleSpecialTransitionsTable->setHorizontalHeaderLabels(QStringList() << tr("Parameter") << tr("Rule special transition"));
	ui_->ruleSpecialTransitionsTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

	vHeader = ui_->ruleSymbolEquationsTable->verticalHeader();
	vHeader->setResizeMode(QHeaderView::Fixed);
	vHeader->setDefaultSectionSize(rowHeight);
	ui_->ruleSymbolEquationsTable->setColumnCount(NUM_RULE_SYMBOL_EQUATIONS_TABLE_COLUMNS);
	ui_->ruleSymbolEquationsTable->setHorizontalHeaderLabels(QStringList() << tr("Symbol") << tr("Rule equation"));
	ui_->ruleSymbolEquationsTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

	ui_->transitionsTree->setColumnCount(NUM_TRANSITIONS_TREE_COLUMNS);
	ui_->transitionsTree->setHeaderLabels(QStringList() << tr("Transition"));

	ui_->specialTransitionsTree->setColumnCount(NUM_SPECIAL_TRANSITIONS_TREE_COLUMNS);
	ui_->specialTransitionsTree->setHeaderLabels(QStringList() << tr("Special transition"));

	ui_->equationsTree->setColumnCount(NUM_EQUATIONS_TREE_COLUMNS);
	ui_->equationsTree->setHeaderLabels(QStringList() << tr("Equation"));
}

RuleEditorWindow::~RuleEditorWindow()
{
}

void
RuleEditorWindow::clear()
{
	clearRuleData();
}

void
RuleEditorWindow::resetModel(TRMControlModel::Model* model)
{
	if (model == nullptr) {
		clear();
		model_ = nullptr;
	} else {
		model_ = model;
		setupTransitionsTree();
		setupSpecialTransitionsTree();
		setupEquationsTree();
	}
}

// Slot.
void
RuleEditorWindow::handleEditRuleButtonClicked(unsigned int ruleIndex)
{
	qDebug("handleEditRuleButtonClicked");

	if (model_ == nullptr) return;

	// Find the rule instance.
	if (ruleIndex < model_->ruleList().size()) {
		rule_ = model_->ruleList()[ruleIndex].get();
	} else {
		rule_ = nullptr;
		return;
	}

	ui_->ruleNumberLabel->setText(QString("<b>%1</b>").arg(ruleIndex + 1));

	setupRuleTransitionsTable();
	setupRuleSpecialTransitionsTable();
	setupRuleSymbolEquationsTable();

	ui_->ruleTransitionsTable->setCurrentItem(nullptr);
	ui_->ruleSpecialTransitionsTable->setCurrentItem(nullptr);
	ui_->ruleSymbolEquationsTable->setCurrentItem(nullptr);

	ui_->transitionsTree->setCurrentItem(nullptr);
	ui_->specialTransitionsTree->setCurrentItem(nullptr);
	ui_->equationsTree->setCurrentItem(nullptr);

	show();
	raise();
}

// Slot.
void
RuleEditorWindow::clearRuleData()
{
	ui_->ruleTransitionsTable->setRowCount(0);
	ui_->ruleSpecialTransitionsTable->setRowCount(0);
	ui_->ruleSymbolEquationsTable->setRowCount(0);

	ui_->ruleNumberLabel->setText(QString("<b>%1</b>").arg(0));
}

void
RuleEditorWindow::on_clearSpecialTransitionButton_clicked()
{
	qDebug("on_clearSpecialTransitionButton_clicked");

	if (model_ == nullptr) return;
	if (rule_ == nullptr) return;

	QTableWidgetItem* currentItem = ui_->ruleSpecialTransitionsTable->currentItem();
	if (currentItem == nullptr) return;

	int row = currentItem->row();

	std::shared_ptr<TRMControlModel::Transition> empty;
	rule_->setSpecialProfileTransition(row, empty);

	setupRuleSpecialTransitionsTable();
	ui_->specialTransitionsTree->setCurrentItem(nullptr);

	emit specialTransitionReferenceChanged();
}

void
RuleEditorWindow::on_clearEquationButton_clicked()
{
	qDebug("on_clearEquationButton_clicked");

	if (model_ == nullptr) return;
	if (rule_ == nullptr) return;

	QTableWidgetItem* currentItem = ui_->ruleSymbolEquationsTable->currentItem();
	if (currentItem == nullptr) return;

	std::shared_ptr<TRMControlModel::Equation>* ruleEquation = nullptr;
	int row = currentItem->row();
	switch (row) {
	case 0:
		ruleEquation = &rule_->exprSymbolEquations().ruleDuration;
		break;
	case 1:
		ruleEquation = &rule_->exprSymbolEquations().beat;
		break;
	case 2:
		ruleEquation = &rule_->exprSymbolEquations().mark1;
		break;
	case 3:
		ruleEquation = &rule_->exprSymbolEquations().mark2;
		break;
	case 4:
		ruleEquation = &rule_->exprSymbolEquations().mark3;
		break;
	default:
		return;
	}

	ruleEquation->reset();

	setupRuleSymbolEquationsTable();
	ui_->equationsTree->setCurrentItem(nullptr);

	emit equationReferenceChanged();
}

void
RuleEditorWindow::on_closeButton_clicked()
{
	hide();
}

void
RuleEditorWindow::on_ruleTransitionsTable_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* /*previous*/)
{
	if (model_ == nullptr) return;
	if (rule_ == nullptr) return;
	if (current == nullptr) return;

	int row = ui_->ruleTransitionsTable->row(current);
	qDebug("on_ruleTransitionsTable_currentItemChanged row=%d", row);

	auto transition = rule_->getParamProfileTransition(row);
	if (transition) {
		unsigned int groupIndex;
		unsigned int transitionIndex;
		if (model_->findTransitionIndex(transition->name(), groupIndex, transitionIndex)) {
			QTreeWidgetItem* topLevelItem = ui_->transitionsTree->topLevelItem(groupIndex);
			if (topLevelItem == nullptr) {
				qCritical("[RuleEditorWindow::on_ruleTransitionsTable_currentItemChanged]: Invalid group index: %u.", groupIndex);
				return;
			}
			QTreeWidgetItem* item = topLevelItem->child(transitionIndex);
			if (item == nullptr) {
				qCritical("[RuleEditorWindow::on_ruleTransitionsTable_currentItemChanged]: Invalid index: %u for groupIndex: %u.", transitionIndex, groupIndex);
				return;
			}
			ui_->transitionsTree->setCurrentItem(item);
		}
	} else {
		ui_->transitionsTree->setCurrentItem(nullptr);
	}
}

void
RuleEditorWindow::on_ruleSpecialTransitionsTable_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* /*previous*/)
{
	if (model_ == nullptr) return;
	if (rule_ == nullptr) return;
	if (current == nullptr) return;

	int row = ui_->ruleSpecialTransitionsTable->row(current);
	qDebug("on_ruleSpecialTransitionsTable_currentItemChanged row=%d", row);

	auto transition = rule_->getSpecialProfileTransition(row);
	if (transition) {
		unsigned int groupIndex;
		unsigned int transitionIndex;
		if (model_->findSpecialTransitionIndex(transition->name(), groupIndex, transitionIndex)) {
			QTreeWidgetItem* topLevelItem = ui_->specialTransitionsTree->topLevelItem(groupIndex);
			if (topLevelItem == nullptr) {
				qCritical("[RuleEditorWindow::on_ruleSpecialTransitionsTable_currentItemChanged]: Invalid group index: %u.", groupIndex);
				return;
			}
			QTreeWidgetItem* item = topLevelItem->child(transitionIndex);
			if (item == nullptr) {
				qCritical("[RuleEditorWindow::on_ruleSpecialTransitionsTable_currentItemChanged]: Invalid index: %u for groupIndex: %u.", transitionIndex, groupIndex);
				return;
			}
			ui_->specialTransitionsTree->setCurrentItem(item);
		}
	} else {
		ui_->specialTransitionsTree->setCurrentItem(nullptr);
	}
}

void
RuleEditorWindow::on_ruleSymbolEquationsTable_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* /*previous*/)
{
	if (model_ == nullptr) return;
	if (rule_ == nullptr) return;
	if (current == nullptr) return;

	int row = ui_->ruleSymbolEquationsTable->row(current);
	qDebug("on_ruleSymbolEquationsTable_currentItemChanged row=%d", row);

	std::shared_ptr<TRMControlModel::Equation> equation;
	switch (row) {
	case 0:
		equation = rule_->exprSymbolEquations().ruleDuration;
		break;
	case 1:
		equation = rule_->exprSymbolEquations().beat;
		break;
	case 2:
		equation = rule_->exprSymbolEquations().mark1;
		break;
	case 3:
		equation = rule_->exprSymbolEquations().mark2;
		break;
	case 4:
		equation = rule_->exprSymbolEquations().mark3;
		break;
	}

	if (equation) {
		unsigned int groupIndex;
		unsigned int equationIndex;
		if (model_->findEquationIndex(equation->name(), groupIndex, equationIndex)) {
			QTreeWidgetItem* topLevelItem = ui_->equationsTree->topLevelItem(groupIndex);
			if (topLevelItem == nullptr) {
				qCritical("[RuleEditorWindow::on_ruleSymbolEquationsTable_currentItemChanged]: Invalid group index: %u.", groupIndex);
				return;
			}
			QTreeWidgetItem* item = topLevelItem->child(equationIndex);
			if (item == nullptr) {
				qCritical("[RuleEditorWindow::on_ruleSymbolEquationsTable_currentItemChanged]: Invalid index: %u for groupIndex: %u.", equationIndex, groupIndex);
				return;
			}
			ui_->equationsTree->setCurrentItem(item);
		}
	} else {
		ui_->equationsTree->setCurrentItem(nullptr);
	}
}

void
RuleEditorWindow::on_transitionsTree_itemClicked(QTreeWidgetItem* item, int /*column*/)
{
	qDebug("on_transitionsTree_itemClicked");

	if (model_ == nullptr) return;
	if (rule_ == nullptr) return;
	if (item == nullptr) return;

	QTreeWidgetItem* parent = item->parent();
	if (parent == nullptr) { // root item
		return;
	}
	int parentIndex = ui_->transitionsTree->indexOfTopLevelItem(parent);
	int index = parent->indexOfChild(item);
	const auto& transition = model_->transitionGroupList()[parentIndex].transitionList[index];

	QTableWidgetItem* currentItem = ui_->ruleTransitionsTable->currentItem();
	if (currentItem == nullptr) return;

	int row = currentItem->row();
	const auto& ruleTransition = rule_->getParamProfileTransition(row);
	if (ruleTransition != transition) {
		rule_->setParamProfileTransition(row, transition);
		setupRuleTransitionsTable();

		emit transitionReferenceChanged();
	}
}

void
RuleEditorWindow::on_specialTransitionsTree_itemClicked(QTreeWidgetItem* item, int /*column*/)
{
	qDebug("on_specialTransitionsTree_itemClicked");

	if (model_ == nullptr) return;
	if (rule_ == nullptr) return;
	if (item == nullptr) return;

	QTreeWidgetItem* parent = item->parent();
	if (parent == nullptr) { // root item
		return;
	}
	int parentIndex = ui_->specialTransitionsTree->indexOfTopLevelItem(parent);
	int index = parent->indexOfChild(item);
	const auto& transition = model_->specialTransitionGroupList()[parentIndex].transitionList[index];

	QTableWidgetItem* currentItem = ui_->ruleSpecialTransitionsTable->currentItem();
	if (currentItem == nullptr) return;

	int row = currentItem->row();
	const auto& ruleTransition = rule_->getSpecialProfileTransition(row);
	if (ruleTransition != transition) {
		rule_->setSpecialProfileTransition(row, transition);
		setupRuleSpecialTransitionsTable();

		emit specialTransitionReferenceChanged();
	}
}

void
RuleEditorWindow::on_equationsTree_itemClicked(QTreeWidgetItem* item, int /*column*/)
{
	qDebug("on_equationsTree_itemClicked");

	if (model_ == nullptr) return;
	if (rule_ == nullptr) return;
	if (item == nullptr) return;

	QTreeWidgetItem* parent = item->parent();
	if (parent == nullptr) { // root item
		return;
	}
	int parentIndex = ui_->equationsTree->indexOfTopLevelItem(parent);
	int index = parent->indexOfChild(item);
	const auto& equation = model_->equationGroupList()[parentIndex].equationList[index];

	QTableWidgetItem* currentItem = ui_->ruleSymbolEquationsTable->currentItem();
	if (currentItem == nullptr) return;

	std::shared_ptr<TRMControlModel::Equation>* ruleEquation = nullptr;
	int row = currentItem->row();
	switch (row) {
	case 0:
		ruleEquation = &rule_->exprSymbolEquations().ruleDuration;
		break;
	case 1:
		ruleEquation = &rule_->exprSymbolEquations().beat;
		break;
	case 2:
		ruleEquation = &rule_->exprSymbolEquations().mark1;
		break;
	case 3:
		ruleEquation = &rule_->exprSymbolEquations().mark2;
		break;
	case 4:
		ruleEquation = &rule_->exprSymbolEquations().mark3;
		break;
	default:
		return;
	}

	if (*ruleEquation != equation) {
		*ruleEquation = equation;
		setupRuleSymbolEquationsTable();

		emit equationReferenceChanged();
	}
}

// Slot.
void
RuleEditorWindow::setupRuleTransitionsTable()
{
	if (model_ == nullptr) return;
	if (rule_ == nullptr) return;

	QTableWidget* table = ui_->ruleTransitionsTable;

	SignalBlocker blocker(table);

	table->setRowCount(model_->parameterList().size());
	for (unsigned int i = 0, size = model_->parameterList().size(); i < size; ++i) {
		const auto& parameter = model_->parameterList()[i];

		std::unique_ptr<QTableWidgetItem> item(new QTableWidgetItem(parameter.name().c_str()));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(i, 0, item.release());

		const auto& transition = rule_->getParamProfileTransition(i);
		item.reset(new QTableWidgetItem(transition ? transition->name().c_str() : ""));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(i, 1, item.release());
	}
}

// Slot.
void
RuleEditorWindow::setupTransitionsTree()
{
	if (model_ == nullptr) return;

	QTreeWidget* tree = ui_->transitionsTree;
	tree->clear();
	for (const auto& group : model_->transitionGroupList()) {
		std::unique_ptr<QTreeWidgetItem> item(new QTreeWidgetItem);
		item->setText(0, group.name.c_str());
		item->setFlags(Qt::ItemIsEnabled);

		for (const auto& transition : group.transitionList) {
			std::unique_ptr<QTreeWidgetItem> childItem(new QTreeWidgetItem);
			childItem->setText(0, transition->name().c_str());
			childItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			item->addChild(childItem.release());
		}

		tree->addTopLevelItem(item.release());
	}
	tree->expandAll();
}

// Slot.
void
RuleEditorWindow::setupRuleSpecialTransitionsTable()
{
	if (model_ == nullptr) return;
	if (rule_ == nullptr) return;

	QTableWidget* table = ui_->ruleSpecialTransitionsTable;

	SignalBlocker blocker(table);

	table->setRowCount(model_->parameterList().size());
	for (unsigned int i = 0, size = model_->parameterList().size(); i < size; ++i) {
		const auto& parameter = model_->parameterList()[i];

		std::unique_ptr<QTableWidgetItem> item(new QTableWidgetItem(parameter.name().c_str()));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(i, 0, item.release());

		const auto& transition = rule_->getSpecialProfileTransition(i);
		item.reset(new QTableWidgetItem(transition ? transition->name().c_str() : ""));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(i, 1, item.release());
	}
}

// Slot.
void
RuleEditorWindow::setupSpecialTransitionsTree()
{
	if (model_ == nullptr) return;

	QTreeWidget* tree = ui_->specialTransitionsTree;
	tree->clear();
	for (const auto& group : model_->specialTransitionGroupList()) {
		std::unique_ptr<QTreeWidgetItem> item(new QTreeWidgetItem);
		item->setText(0, group.name.c_str());
		item->setFlags(Qt::ItemIsEnabled);

		for (const auto& specialTransition : group.transitionList) {
			std::unique_ptr<QTreeWidgetItem> childItem(new QTreeWidgetItem);
			childItem->setText(0, specialTransition->name().c_str());
			childItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			item->addChild(childItem.release());
		}

		tree->addTopLevelItem(item.release());
	}
	tree->expandAll();
}

// Slot.
void
RuleEditorWindow::setupRuleSymbolEquationsTable()
{
	if (model_ == nullptr) return;
	if (rule_ == nullptr) return;

	QTableWidget* table = ui_->ruleSymbolEquationsTable;

	SignalBlocker blocker(table);

	table->setRowCount(5);
	{
		std::unique_ptr<QTableWidgetItem> item(new QTableWidgetItem("Rule duration"));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(0, 0, item.release());

		const auto& equation = rule_->exprSymbolEquations().ruleDuration;
		item.reset(new QTableWidgetItem(equation ? equation->name().c_str() : ""));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(0, 1, item.release());
	}
	{
		std::unique_ptr<QTableWidgetItem> item(new QTableWidgetItem("Beat location"));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(1, 0, item.release());

		const auto& equation = rule_->exprSymbolEquations().beat;
		item.reset(new QTableWidgetItem(equation ? equation->name().c_str() : ""));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(1, 1, item.release());
	}
	{
		std::unique_ptr<QTableWidgetItem> item(new QTableWidgetItem("Mark 1"));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(2, 0, item.release());

		const auto& equation = rule_->exprSymbolEquations().mark1;
		item.reset(new QTableWidgetItem(equation ? equation->name().c_str() : ""));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(2, 1, item.release());
	}
	{
		std::unique_ptr<QTableWidgetItem> item(new QTableWidgetItem("Mark 2"));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(3, 0, item.release());

		const auto& equation = rule_->exprSymbolEquations().mark2;
		item.reset(new QTableWidgetItem(equation ? equation->name().c_str() : ""));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(3, 1, item.release());
	}
	{
		std::unique_ptr<QTableWidgetItem> item(new QTableWidgetItem("Mark 3"));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(4, 0, item.release());

		const auto& equation = rule_->exprSymbolEquations().mark3;
		item.reset(new QTableWidgetItem(equation ? equation->name().c_str() : ""));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		table->setItem(4, 1, item.release());
	}
}

// Slot.
void
RuleEditorWindow::setupEquationsTree()
{
	if (model_ == nullptr) return;

	QTreeWidget* tree = ui_->equationsTree;
	tree->clear();
	for (const auto& group : model_->equationGroupList()) {
		std::unique_ptr<QTreeWidgetItem> item(new QTreeWidgetItem);
		item->setText(0, group.name.c_str());
		item->setFlags(Qt::ItemIsEnabled);

		for (const auto& equation : group.equationList) {
			std::unique_ptr<QTreeWidgetItem> childItem(new QTreeWidgetItem);
			childItem->setText(0, equation->name().c_str());
			childItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			item->addChild(childItem.release());
		}

		tree->addTopLevelItem(item.release());
	}
	tree->expandAll();
}

} // namespace GS
