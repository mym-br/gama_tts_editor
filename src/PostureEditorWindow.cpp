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

#include "PostureEditorWindow.h"

#include <algorithm> /* find_if */
#include <memory>
#include <string>
#include <utility> /* move */

#include <QMessageBox>

#include "Model.h"
#include "SignalBlocker.h"
#include "ui_PostureEditorWindow.h"

#define NUM_POSTURES_TABLE_COLUMNS 1
#define NUM_CATEGORIES_TABLE_COLUMNS 2
#define NUM_PARAMETERS_TABLE_COLUMNS 5
#define NUM_SYMBOLS_TABLE_COLUMNS 5
#define NEW_ITEM_NAME "___new___"



namespace GS {

PostureEditorWindow::PostureEditorWindow(QWidget* parent)
		: QWidget(parent)
		, ui_(new Ui::PostureEditorWindow)
		, model_(nullptr)
{
	ui_->setupUi(this);

	QFontMetrics fm = fontMetrics();
	int rowHeight = fm.height() + fm.xHeight();

	QHeaderView* vHeader = ui_->posturesTable->verticalHeader();
	vHeader->setResizeMode(QHeaderView::Fixed);
	vHeader->setDefaultSectionSize(rowHeight);
	ui_->posturesTable->setColumnCount(NUM_POSTURES_TABLE_COLUMNS);
	ui_->posturesTable->setHorizontalHeaderLabels(QStringList() << tr("Posture"));
	ui_->posturesTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

	vHeader = ui_->categoriesTable->verticalHeader();
	vHeader->setResizeMode(QHeaderView::Fixed);
	vHeader->setDefaultSectionSize(rowHeight);
	ui_->categoriesTable->setColumnCount(NUM_CATEGORIES_TABLE_COLUMNS);
	ui_->categoriesTable->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Is member?"));

	vHeader = ui_->parametersTable->verticalHeader();
	vHeader->setResizeMode(QHeaderView::Fixed);
	vHeader->setDefaultSectionSize(rowHeight);
	ui_->parametersTable->setColumnCount(NUM_PARAMETERS_TABLE_COLUMNS);
	ui_->parametersTable->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value") << tr("Minimum") << tr("Maximum") << tr("Default"));
	ui_->parametersTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

	vHeader = ui_->symbolsTable->verticalHeader();
	vHeader->setResizeMode(QHeaderView::Fixed);
	vHeader->setDefaultSectionSize(rowHeight);
	ui_->symbolsTable->setColumnCount(NUM_SYMBOLS_TABLE_COLUMNS);
	ui_->symbolsTable->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value") << tr("Minimum") << tr("Maximum") << tr("Default"));
	ui_->symbolsTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
}

PostureEditorWindow::~PostureEditorWindow()
{
}

void
PostureEditorWindow::resetModel(TRMControlModel::Model* model)
{
	model_ = model;

	if (model_ == nullptr) {
		clearPostureData();
		ui_->posturesTable->setRowCount(0);
	} else {
		model_->sortPostures();
		setupPosturesTable();
	}
}

void
PostureEditorWindow::on_addPostureButton_clicked()
{
	if (model_ == nullptr) return;

	if (model_->findPostureName(NEW_ITEM_NAME)) {
		qWarning("Duplicate posture name.");
		return;
	}

	std::unique_ptr<TRMControlModel::Posture> newPosture(new TRMControlModel::Posture(model_->parameterList().size(), model_->symbolList().size()));
	newPosture->setName(NEW_ITEM_NAME);
	std::shared_ptr<TRMControlModel::Category> cat = model_->findCategory("phone"); // hardcoded
	if (!cat) {
		QMessageBox::critical(this, tr("Error"), "Category not found: phone.");
		return;
	}
	newPosture->categoryList().push_back(cat);
	for (unsigned int i = 0, size = model_->parameterList().size(); i < size; ++i) {
		const auto& parameter = model_->parameterList()[i];
		newPosture->setParameterTarget(i, parameter.defaultValue());
	}
	for (unsigned int i = 0, size = model_->symbolList().size(); i < size; ++i) {
		const auto& symbol = model_->symbolList()[i];
		newPosture->setSymbolTarget(i, symbol.defaultValue());
	}
	model_->postureList().push_back(std::move(newPosture));

	model_->sortPostures();
	setupPosturesTable();
}

void
PostureEditorWindow::on_removePostureButton_clicked()
{
	if (model_ == nullptr) return;

	QTableWidgetItem* currPostureItem = ui_->posturesTable->currentItem();
	if (currPostureItem == nullptr) return;

	int currPostureRow = currPostureItem->row();
	model_->postureList().erase(model_->postureList().begin() + currPostureRow);

	setupPosturesTable();
}

void
PostureEditorWindow::on_updatePostureCommentButton_clicked()
{
	if (model_ == nullptr) return;

	QTableWidgetItem* currPostureItem = ui_->posturesTable->currentItem();
	if (currPostureItem == nullptr) return;

	int currPostureRow = currPostureItem->row();
	TRMControlModel::Posture& posture = *model_->postureList()[currPostureRow];
	posture.setComment(ui_->postureCommentTextEdit->toPlainText().toStdString());
}

void
PostureEditorWindow::on_posturesTable_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* /*previous*/)
{
	if (model_ == nullptr) return;
	if (current == nullptr) {
		clearPostureData();
		return;
	}

	unsigned int row = current->row();
	const TRMControlModel::Posture& posture = *model_->postureList()[row];
	ui_->postureCommentTextEdit->setPlainText(posture.comment().c_str());
	setupCategoriesTable(posture);
	setupParametersTable(posture);
	setupSymbolsTable(posture);
}

void
PostureEditorWindow::on_posturesTable_itemChanged(QTableWidgetItem* item)
{
	qDebug("on_posturesTable_itemChanged");

	if (model_ == nullptr) return;

	int row = item->row();
	const std::string newName = item->text().toStdString();
	if (model_->findPostureName(newName)) {
		qWarning("Duplicate posture name.");
		{
			SignalBlocker blocker(ui_->posturesTable);
			item->setText(model_->postureList()[row]->name().c_str());
		}
	} else if (model_->findCategoryName(newName)) {
		qWarning("Posture can not have the same name as a category.");
		{
			SignalBlocker blocker(ui_->posturesTable);
			item->setText(model_->postureList()[row]->name().c_str());
		}
	} else {
		auto& posture = *model_->postureList()[row];
		std::string oldName = posture.name();
		auto iter = std::find_if(
				posture.categoryList().begin(), posture.categoryList().end(),
				[&oldName](const std::shared_ptr<TRMControlModel::Category>& c) -> bool {
					return c->native() && c->name() == oldName;
				});
		if (iter != posture.categoryList().end()) {
			posture.setName(newName);
			(*iter)->setName(newName);
		} else {
			qCritical("[PostureEditorWindow::on_posturesTable_itemChanged] Posture does not have category with the same name as the posture.");
		}
		model_->sortPostures();
		setupPosturesTable();
	}
}

void
PostureEditorWindow::on_categoriesTable_itemChanged(QTableWidgetItem* item)
{
	qDebug("on_categoriesTable_itemChanged");

	if (model_ == nullptr) return;

	QTableWidgetItem* currPostureItem = ui_->posturesTable->currentItem();
	if (currPostureItem == nullptr) return;
	int currPostureRow = currPostureItem->row();

	int row = item->row();
	int col = item->column();
	if (col == 1) {
		TRMControlModel::Posture& posture = *model_->postureList()[currPostureRow];
		std::shared_ptr<TRMControlModel::Category> category = model_->categoryList()[row];
		if (item->checkState() == Qt::Checked) {
			posture.categoryList().push_back(category);
		} else {
			auto iter = std::find_if(
					posture.categoryList().begin(), posture.categoryList().end(),
					[&category](const std::shared_ptr<TRMControlModel::Category>& c) {
						return c->name() == category->name();
					});
			if (iter != posture.categoryList().end()) {
				posture.categoryList().erase(iter);
			}
		}
	}
}

void
PostureEditorWindow::on_parametersTable_itemChanged(QTableWidgetItem* item)
{
	qDebug("on_parametersTable_itemChanged");

	if (model_ == nullptr) return;

	QTableWidgetItem* currPostureItem = ui_->posturesTable->currentItem();
	if (currPostureItem == nullptr) return;
	int currPostureRow = currPostureItem->row();

	int row = item->row();
	int col = item->column();
	if (col == 1) {
		TRMControlModel::Posture& posture = *model_->postureList()[currPostureRow];
		const TRMControlModel::Parameter& parameter = model_->parameterList()[row];

		bool ok;
		float newValue = item->data(Qt::DisplayRole).toFloat(&ok);
		bool acceptValue = true;
		if (!ok) {
			newValue = posture.getParameterTarget(row);
			acceptValue = false;
		}
		if (newValue < parameter.minimum()) {
			newValue = parameter.minimum();
			acceptValue = false;

		} else if (newValue > parameter.maximum()) {
			newValue = parameter.maximum();
			acceptValue = false;
		}
		if (acceptValue) {
			posture.setParameterTarget(row, newValue);
		} else {
			SignalBlocker blocker(ui_->parametersTable);
			item->setData(Qt::DisplayRole, newValue);
		}
	}
}

void
PostureEditorWindow::on_useDefaultParameterValueButton_clicked()
{
	if (model_ == nullptr) return;

	QTableWidgetItem* currPostureItem = ui_->posturesTable->currentItem();
	if (currPostureItem == nullptr) return;
	int currPostureRow = currPostureItem->row();
	TRMControlModel::Posture& posture = *model_->postureList()[currPostureRow];

	QTableWidgetItem* item = ui_->parametersTable->currentItem();
	if (item == nullptr) return;
	int row = item->row();
	const TRMControlModel::Parameter& parameter = model_->parameterList()[row];

	posture.setParameterTarget(row, parameter.defaultValue());
	setupParametersTable(posture);
}

void
PostureEditorWindow::on_symbolsTable_itemChanged(QTableWidgetItem* item)
{
	qDebug("on_symbolsTable_itemChanged");

	if (model_ == nullptr) return;

	QTableWidgetItem* currPostureItem = ui_->posturesTable->currentItem();
	if (currPostureItem == nullptr) return;
	int currPostureRow = currPostureItem->row();

	int row = item->row();
	int col = item->column();
	if (col == 1) {
		TRMControlModel::Posture& posture = *model_->postureList()[currPostureRow];
		const TRMControlModel::Symbol& symbol = model_->symbolList()[row];

		bool ok;
		float newValue = item->data(Qt::DisplayRole).toFloat(&ok);
		bool acceptValue = true;
		if (!ok) {
			newValue = posture.getSymbolTarget(row);
			acceptValue = false;
		}
		if (newValue < symbol.minimum()) {
			newValue = symbol.minimum();
			acceptValue = false;

		} else if (newValue > symbol.maximum()) {
			newValue = symbol.maximum();
			acceptValue = false;
		}
		if (acceptValue) {
			posture.setSymbolTarget(row, newValue);
		} else {
			SignalBlocker blocker(ui_->symbolsTable);
			item->setData(Qt::DisplayRole, newValue);
		}
	}
}

void
PostureEditorWindow::on_useDefaultSymbolValueButton_clicked()
{
	if (model_ == nullptr) return;

	QTableWidgetItem* currPostureItem = ui_->posturesTable->currentItem();
	if (currPostureItem == nullptr) return;
	int currPostureRow = currPostureItem->row();
	TRMControlModel::Posture& posture = *model_->postureList()[currPostureRow];

	QTableWidgetItem* item = ui_->symbolsTable->currentItem();
	if (item == nullptr) return;
	int row = item->row();
	const TRMControlModel::Symbol& symbol = model_->symbolList()[row];

	posture.setSymbolTarget(row, symbol.defaultValue());
	setupSymbolsTable(posture);
}

void
PostureEditorWindow::setupPosturesTable()
{
	if (model_ == nullptr) return;

	QTableWidget* table = ui_->posturesTable;
	{
		SignalBlocker blocker(table);

		table->setRowCount(model_->postureList().size());
		for (unsigned int i = 0, size = model_->postureList().size(); i < size; ++i) {
			const auto& posture = model_->postureList()[i];

			std::unique_ptr<QTableWidgetItem> item(new QTableWidgetItem(posture->name().c_str()));
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
			table->setItem(i, 0, item.release());
		}
	}
	table->setCurrentItem(nullptr);
}

void
PostureEditorWindow::clearPostureData()
{
	ui_->postureCommentTextEdit->clear();
	ui_->categoriesTable->setRowCount(0);
	ui_->parametersTable->setRowCount(0);
	ui_->symbolsTable->setRowCount(0);
}

void
PostureEditorWindow::setupCategoriesTable(const TRMControlModel::Posture& posture)
{
	QTableWidget* table = ui_->categoriesTable;
	{
		SignalBlocker blocker(table);

		table->setRowCount(model_->categoryList().size());
		for (unsigned int i = 0, size = model_->categoryList().size(); i < size; ++i) {
			const auto& category = model_->categoryList()[i];

			std::unique_ptr<QTableWidgetItem> item(new QTableWidgetItem(category->name().c_str()));
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			table->setItem(i, 0, item.release());

			item.reset(new QTableWidgetItem);
			if (category->name() == "phone") { // hardcoded
				item->setFlags(Qt::ItemIsSelectable);
			} else {
				item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
			}
			bool useCategory = false;
			for (auto& postureCategory : posture.categoryList()) {
				if (category->name() == postureCategory->name()) {
					useCategory = true;
					break;
				}
			}
			item->setCheckState(useCategory ? Qt::Checked : Qt::Unchecked);
			table->setItem(i, 1, item.release());
		}
	}
}

void
PostureEditorWindow::setupParametersTable(const TRMControlModel::Posture& posture)
{
	QTableWidget* table = ui_->parametersTable;
	{
		SignalBlocker blocker(table);

		QFont boldFont = font();
		boldFont.setBold(true);

		table->setRowCount(model_->parameterList().size());
		for (unsigned int i = 0, size = model_->parameterList().size(); i < size; ++i) {
			const auto& parameter = model_->parameterList()[i];

			float value = posture.getParameterTarget(i);
			float defaultValue = parameter.defaultValue();
			bool usingDefaultValue = (value == defaultValue);

			// Name.
			std::unique_ptr<QTableWidgetItem> item(new QTableWidgetItem(parameter.name().c_str()));
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			if (usingDefaultValue) item->setData(Qt::FontRole, boldFont);
			table->setItem(i, 0, item.release());

			// Value.
			item.reset(new QTableWidgetItem);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
			if (usingDefaultValue) item->setData(Qt::FontRole, boldFont);
			item->setData(Qt::DisplayRole, value);
			table->setItem(i, 1, item.release());

			// Min.
			item.reset(new QTableWidgetItem);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			if (usingDefaultValue) item->setData(Qt::FontRole, boldFont);
			item->setData(Qt::DisplayRole, parameter.minimum());
			table->setItem(i, 2, item.release());

			// Max.
			item.reset(new QTableWidgetItem);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			if (usingDefaultValue) item->setData(Qt::FontRole, boldFont);
			item->setData(Qt::DisplayRole, parameter.maximum());
			table->setItem(i, 3, item.release());

			// Default.
			item.reset(new QTableWidgetItem);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			if (usingDefaultValue) item->setData(Qt::FontRole, boldFont);
			item->setData(Qt::DisplayRole, defaultValue);
			table->setItem(i, 4, item.release());
		}
	}
}

void
PostureEditorWindow::setupSymbolsTable(const TRMControlModel::Posture& posture)
{
	QTableWidget* table = ui_->symbolsTable;
	{
		SignalBlocker blocker(table);

		QFont boldFont = font();
		boldFont.setBold(true);

		table->setRowCount(model_->symbolList().size());
		for (unsigned int i = 0, size = model_->symbolList().size(); i < size; ++i) {
			const auto& symbol = model_->symbolList()[i];

			float value = posture.getSymbolTarget(i);
			float defaultValue = symbol.defaultValue();
			bool usingDefaultValue = (value == defaultValue);

			// Name.
			std::unique_ptr<QTableWidgetItem> item(new QTableWidgetItem(symbol.name().c_str()));
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			if (usingDefaultValue) item->setData(Qt::FontRole, boldFont);
			table->setItem(i, 0, item.release());

			// Value.
			item.reset(new QTableWidgetItem);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
			if (usingDefaultValue) item->setData(Qt::FontRole, boldFont);
			item->setData(Qt::DisplayRole, value);
			table->setItem(i, 1, item.release());

			// Min.
			item.reset(new QTableWidgetItem);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			if (usingDefaultValue) item->setData(Qt::FontRole, boldFont);
			item->setData(Qt::DisplayRole, symbol.minimum());
			table->setItem(i, 2, item.release());

			// Max.
			item.reset(new QTableWidgetItem);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			if (usingDefaultValue) item->setData(Qt::FontRole, boldFont);
			item->setData(Qt::DisplayRole, symbol.maximum());
			table->setItem(i, 3, item.release());

			// Default.
			item.reset(new QTableWidgetItem);
			item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			if (usingDefaultValue) item->setData(Qt::FontRole, boldFont);
			item->setData(Qt::DisplayRole, defaultValue);
			table->setItem(i, 4, item.release());
		}
	}
}

} // namespace GS
