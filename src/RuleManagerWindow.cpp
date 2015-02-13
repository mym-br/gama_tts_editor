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

#include "RuleManagerWindow.h"

#include <memory>
#include <utility> /* move, swap */

#include <QMessageBox>

#include "Model.h"
#include "SignalBlocker.h"
#include "ui_RuleManagerWindow.h"

#define NUM_RULES_TABLE_COLUMNS 1



namespace GS {

RuleManagerWindow::RuleManagerWindow(QWidget* parent)
		: QWidget(parent)
		, ui_(new Ui::RuleManagerWindow)
		, model_(nullptr)
{
	ui_->setupUi(this);

	QFontMetrics fm = fontMetrics();
	int rowHeight = fm.height() + fm.xHeight();

	QHeaderView* vHeader = ui_->rulesTable->verticalHeader();
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	vHeader->setSectionResizeMode(QHeaderView::Fixed);
#else
	vHeader->setResizeMode(QHeaderView::Fixed);
#endif
	vHeader->setDefaultSectionSize(rowHeight);
	ui_->rulesTable->setColumnCount(NUM_RULES_TABLE_COLUMNS);
	ui_->rulesTable->setHorizontalHeaderLabels(QStringList() << tr("Rule"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	ui_->rulesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
	ui_->rulesTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif
}

RuleManagerWindow::~RuleManagerWindow()
{
}

void
RuleManagerWindow::resetModel(TRMControlModel::Model* model)
{
	model_ = model;

	if (model_ == nullptr) {
		clearRuleData();
		ui_->rulesTable->setRowCount(0);
	} else {
		setupRulesList();
		ui_->rulesTable->setCurrentItem(nullptr);
	}
}

void
RuleManagerWindow::on_removeButton_clicked()
{
	if (model_ == nullptr) return;

	QTableWidgetItem* currItem = ui_->rulesTable->currentItem();
	if (currItem == nullptr) return;

	int currRow = currItem->row();
	model_->ruleList().erase(model_->ruleList().begin() + currRow);

	setupRulesList();
	ui_->rulesTable->setCurrentItem(nullptr);
}

void
RuleManagerWindow::on_addButton_clicked()
{
	if (model_ == nullptr) return;

	QString exp1 = ui_->expression1LineEdit->text().trimmed();
	QString exp2 = ui_->expression2LineEdit->text().trimmed();
	QString exp3 = ui_->expression3LineEdit->text().trimmed();
	QString exp4 = ui_->expression4LineEdit->text().trimmed();

	unsigned int numExpr = numInputExpressions(exp1, exp2, exp3, exp4);
	if (numExpr == 0) {
		QMessageBox::critical(this, tr("Error"), "Wrong number of expressions.");
		return;
	}

	std::unique_ptr<TRMControlModel::Rule> newRule(
			new TRMControlModel::Rule(model_->parameterList().size()));

	std::vector<std::string> exprList;
	exprList.push_back(exp1.toStdString());
	exprList.push_back(exp2.toStdString());
	if (numExpr >= 3) {
		exprList.push_back(exp3.toStdString());
	}
	if (numExpr == 4) {
		exprList.push_back(exp4.toStdString());
	}
	try {
		newRule->setBooleanExpressionList(exprList, *model_);
	} catch (const Exception& exc) {
		QMessageBox::critical(this, tr("Error"), exc.what());
		return;
	}

	QTableWidgetItem* currItem = ui_->rulesTable->currentItem();
	unsigned int insertRow;
	if (currItem == nullptr) {
		insertRow = model_->ruleList().size();
	} else {
		insertRow = currItem->row();
	}
	model_->ruleList().insert(model_->ruleList().begin() + insertRow, std::move(newRule));

	setupRulesList();

	// Force the emission of signal currentItemChanged.
	ui_->rulesTable->setCurrentItem(nullptr);
	ui_->rulesTable->setCurrentCell(insertRow, 0);
}

void
RuleManagerWindow::on_updateButton_clicked()
{
	qDebug("on_updateButton_clicked");

	if (model_ == nullptr) return;

	QTableWidgetItem* currItem = ui_->rulesTable->currentItem();
	if (currItem == nullptr) return;

	int currRow = currItem->row();
	TRMControlModel::Rule& rule = *model_->ruleList()[currRow];

	QString exp1 = ui_->expression1LineEdit->text().trimmed();
	QString exp2 = ui_->expression2LineEdit->text().trimmed();
	QString exp3 = ui_->expression3LineEdit->text().trimmed();
	QString exp4 = ui_->expression4LineEdit->text().trimmed();

	unsigned int numExpr = numInputExpressions(exp1, exp2, exp3, exp4);
	if (numExpr == 0) {
		QMessageBox::critical(this, tr("Error"), "Wrong number of expressions.");
		return;
	}

	std::vector<std::string> exprList;
	exprList.push_back(exp1.toStdString());
	exprList.push_back(exp2.toStdString());
	if (numExpr >= 3) {
		exprList.push_back(exp3.toStdString());
	}
	if (numExpr == 4) {
		exprList.push_back(exp4.toStdString());
	}
	try {
		rule.setBooleanExpressionList(exprList, *model_);
	} catch (const Exception& exc) {
		QMessageBox::critical(this, tr("Error"), exc.what());
	}

	setupRulesList();

	// Force the emission of signal currentItemChanged.
	ui_->rulesTable->setCurrentItem(nullptr);
	ui_->rulesTable->setCurrentCell(currRow, 0);

	emit categoryReferenceChanged();
}

void
RuleManagerWindow::on_moveUpButton_clicked()
{
	if (model_ == nullptr) return;

	QTableWidgetItem* currItem = ui_->rulesTable->currentItem();
	if (currItem == nullptr) return;

	int currRow = currItem->row();
	if (currRow == 0) return;

	std::swap(model_->ruleList()[currRow], model_->ruleList()[currRow - 1]);

	setupRulesList();
	ui_->rulesTable->setCurrentCell(currRow - 1, 0);
}

void
RuleManagerWindow::on_moveDownButton_clicked()
{
	if (model_ == nullptr) return;

	QTableWidgetItem* currItem = ui_->rulesTable->currentItem();
	if (currItem == nullptr) return;

	int currRow = currItem->row();
	if (currRow == static_cast<int>(model_->ruleList().size()) - 1) return;

	std::swap(model_->ruleList()[currRow], model_->ruleList()[currRow + 1]);

	setupRulesList();
	ui_->rulesTable->setCurrentCell(currRow + 1, 0);
}

void
RuleManagerWindow::on_editButton_clicked()
{
	if (model_ == nullptr) return;

	QTableWidgetItem* currItem = ui_->rulesTable->currentItem();
	if (currItem == nullptr) return;

	int currRow = currItem->row();
	emit editRuleButtonClicked(currRow);
}

void
RuleManagerWindow::on_rulesTable_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* /*previous*/)
{
	qDebug("on_rulesTable_currentItemChanged");

	if (model_ == nullptr) return;
	if (current == nullptr) {
		clearRuleData();
		return;
	}

	unsigned int row = current->row();
	const TRMControlModel::Rule& rule = *model_->ruleList()[row];

	clearRuleData();

	unsigned int numExpr = rule.booleanExpressionList().size();
	if (numExpr >= 1) ui_->expression1LineEdit->setText(rule.booleanExpressionList()[0].c_str());
	if (numExpr >= 2) ui_->expression2LineEdit->setText(rule.booleanExpressionList()[1].c_str());
	if (numExpr >= 3) ui_->expression3LineEdit->setText(rule.booleanExpressionList()[2].c_str());
	if (numExpr == 4) ui_->expression4LineEdit->setText(rule.booleanExpressionList()[3].c_str());

	showRuleStatistics(rule);
}

// Slot.
void
RuleManagerWindow::clearRuleData()
{
	ui_->expression1LineEdit->clear();
	ui_->expression2LineEdit->clear();
	ui_->expression3LineEdit->clear();
	ui_->expression4LineEdit->clear();
	ui_->matches1Label->setText("Total matches: 0");
	ui_->matches2Label->setText("Total matches: 0");
	ui_->matches3Label->setText("Total matches: 0");
	ui_->matches4Label->setText("Total matches: 0");
	ui_->matches1ListWidget->clear();
	ui_->matches2ListWidget->clear();
	ui_->matches3ListWidget->clear();
	ui_->matches4ListWidget->clear();
	ui_->combinationsLineEdit->clear();
}

// Slot.
void
RuleManagerWindow::unselectRule()
{
	ui_->rulesTable->setCurrentItem(nullptr);
}

void
RuleManagerWindow::setupRulesList()
{
	if (model_ == nullptr) return;

	QTableWidget* table = ui_->rulesTable;
	{
		SignalBlocker blocker(table);

		table->setRowCount(model_->ruleList().size());
		for (unsigned int i = 0, size = model_->ruleList().size(); i < size; ++i) {
			const auto& rule = model_->ruleList()[i];

			QString ruleText;
			for (unsigned int j = 0, size = rule->booleanExpressionList().size(); j < size; ++j) {
				if (j > 0) {
					ruleText += " >> ";
				}
				ruleText += rule->booleanExpressionList()[j].c_str();
			}

			std::unique_ptr<QTableWidgetItem> item(new QTableWidgetItem(ruleText));
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			table->setItem(i, 0, item.release());
		}
	}
}

void
RuleManagerWindow::showRuleStatistics(const TRMControlModel::Rule& rule)
{
	unsigned int count1 = 0;
	unsigned int count2 = 0;
	unsigned int count3 = 0;
	unsigned int count4 = 0;

	for (unsigned int i = 0, size = model_->postureList().size(); i < size; ++i) {
		const auto& posture = model_->postureList()[i];

		if (rule.evalBooleanExpression(posture, 0)) {
			++count1;
			ui_->matches1ListWidget->addItem(posture.name().c_str());
		}
		if (rule.evalBooleanExpression(posture, 1)) {
			++count2;
			ui_->matches2ListWidget->addItem(posture.name().c_str());
		}
		if (rule.evalBooleanExpression(posture, 2)) {
			++count3;
			ui_->matches3ListWidget->addItem(posture.name().c_str());
		}
		if (rule.evalBooleanExpression(posture, 3)) {
			++count4;
			ui_->matches4ListWidget->addItem(posture.name().c_str());
		}
	}

	ui_->matches1Label->setText(QString("Total matches: %1").arg(count1));
	ui_->matches2Label->setText(QString("Total matches: %1").arg(count2));
	ui_->matches3Label->setText(QString("Total matches: %1").arg(count3));
	ui_->matches4Label->setText(QString("Total matches: %1").arg(count4));

	double combinations = 1.0;
	if (count1 != 0) {
		combinations *= static_cast<double>(count1);
	}
	if (count2 != 0) {
		combinations *= static_cast<double>(count2);
	}
	if (count3 != 0) {
		combinations *= static_cast<double>(count3);
	}
	if (count4 != 0) {
		combinations *= static_cast<double>(count4);
	}

	ui_->combinationsLineEdit->setText(QString::number(combinations));
}

unsigned int
RuleManagerWindow::numInputExpressions(const QString& exp1, const QString& exp2, const QString& exp3, const QString& exp4)
{
	if (!exp1.isEmpty() && !exp2.isEmpty()) {
		if (!exp3.isEmpty()) {
			if (!exp4.isEmpty()) {
				return 4;
			} else {
				return 3;
			}
		} else if (exp4.isEmpty()) {
			return 2;
		}
	}
	return 0;
}

} // namespace GS
