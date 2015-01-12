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

#include "SymbolModel.h"

#include <cstddef> /* std::size_t */
#include <utility> /* swap */

#include "Model.h"

#define NEW_ITEM_NAME "___new___"



namespace GS {

SymbolModel::SymbolModel(QObject* parent)
		: QAbstractTableModel(parent)
		, model_(nullptr)
{
}

int
SymbolModel::rowCount(const QModelIndex& /*parent*/) const
{
	return model_ == nullptr ? 0 : model_->symbolList().size();
}

int
SymbolModel::columnCount(const QModelIndex& /*parent*/) const
{
	return NUM_COLUMNS;
}

QVariant
SymbolModel::data(const QModelIndex& index, int role) const
{
	if (model_ == nullptr || !index.isValid()) {
		return QVariant();
	}
	unsigned int row = index.row();
	if (row >= model_->symbolList().size()) {
		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole) {
		switch (index.column()) {
		case 0:
			return model_->symbolList()[row].name().c_str();
			break;
		case 1:
			return model_->symbolList()[row].minimum();
			break;
		case 2:
			return model_->symbolList()[row].maximum();
			break;
		case 3:
			return model_->symbolList()[row].defaultValue();
			break;
		}
	}
	return QVariant();
}

QModelIndex
SymbolModel::index(int row, int column, const QModelIndex& /*parent*/) const
{
	return createIndex(row, column);
}

QModelIndex
SymbolModel::parent(const QModelIndex& /*index*/) const
{
	return QModelIndex();
}

Qt::ItemFlags
SymbolModel::flags(const QModelIndex& index) const
{
	if (!index.isValid()) {
		return Qt::ItemIsEnabled;
	}

	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool
SymbolModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (model_ == nullptr || !index.isValid() || role != Qt::EditRole) {
		return false;
	}
	unsigned int row = index.row();
	if (row >= model_->symbolList().size()) {
		return false;
	}

	switch (index.column()) {
	case 0:
		{
			std::string name = value.toString().toStdString();
			if (model_->findSymbolName(name)) {
				qWarning("Duplicate symbol: %s", name.c_str());
				return false;
			}
			model_->symbolList()[row].setName(name);
			emit dataChanged(index, index);
			return true;
		}
	case 1:
		model_->symbolList()[row].setMinimum(value.toFloat());
		emit dataChanged(index, index);
		return true;
	case 2:
		model_->symbolList()[row].setMaximum(value.toFloat());
		emit dataChanged(index, index);
		return true;
	case 3:
		model_->symbolList()[row].setDefaultValue(value.toFloat());
		emit dataChanged(index, index);
		return true;
	default:
		return false;
	}
}

QVariant
SymbolModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole) {
		return QVariant();
	}

	if (orientation == Qt::Horizontal) {
		switch (section) {
		case 0:
			return QVariant(tr("Name"));
		case 1:
			return QVariant(tr("Minimum"));
		case 2:
			return QVariant(tr("Maximum"));
		case 3:
			return QVariant(tr("Default"));
		default:
			return QVariant();
		}
	} else {
		return QVariant(section + 1);
	}
}

bool
SymbolModel::insertRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (model_ == nullptr || count != 1) {
		return false;
	}
	if (row < 0 || static_cast<std::size_t>(row) > model_->symbolList().size()) {
		return false;
	}

	if (model_->findSymbolName(NEW_ITEM_NAME)) {
		qWarning("Duplicate symbol: %s", NEW_ITEM_NAME);
		return false;
	}

	beginInsertRows(QModelIndex(), row, row);
	model_->symbolList().insert(
				model_->symbolList().begin() + row,
				TRMControlModel::Symbol(NEW_ITEM_NAME, 0.0, 0.0, 0.0));
	endInsertRows();

	return true;
}

bool
SymbolModel::removeRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (model_ == nullptr || count != 1) {
		return false;
	}
	if (row < 0 || static_cast<std::size_t>(row) >= model_->symbolList().size()) {
		return false;
	}

	beginRemoveRows(QModelIndex(), row, row);
	model_->symbolList().erase(model_->symbolList().begin() + row);
	endRemoveRows();

	return true;
}

void
SymbolModel::resetModel(TRMControlModel::Model* model)
{
	beginResetModel();
	model_ = model;
	endResetModel();
}

// Returns the new index.
QModelIndex
SymbolModel::incrementSymbolRow(const QModelIndex& index)
{
	if (model_ == nullptr || !index.isValid()) {
		return index;
	}

	unsigned int row = index.row();
	if (row < model_->symbolList().size() - 1U) {
		std::swap(model_->symbolList()[row], model_->symbolList()[row + 1]);
		emit dataChanged(createIndex(row, 0 /* first column */), createIndex(row + 1, NUM_COLUMNS - 1 /* last column */));
		return createIndex(row + 1, NUM_COLUMNS - 1 /* last column */);
	}
	return index;
}

// Returns the new index.
QModelIndex
SymbolModel::decrementSymbolRow(const QModelIndex& index)
{
	if (model_ == nullptr || !index.isValid()) {
		return index;
	}

	unsigned int row = index.row();
	std::swap(model_->symbolList()[row - 1], model_->symbolList()[row]);
	emit dataChanged(createIndex(row - 1, 0 /* first column */), createIndex(row, NUM_COLUMNS - 1 /* last column */));
	return createIndex(row - 1, NUM_COLUMNS - 1 /* last column */);
}

} // namespace GS
