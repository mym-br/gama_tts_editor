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

#include "ParameterModel.h"

#include <cstddef> /* std::size_t */
#include <utility> /* swap */

#include "Model.h"

#define NEW_ITEM_NAME "___new___"



namespace GS {

ParameterModel::ParameterModel(QObject* parent)
		: QAbstractTableModel(parent)
		, model_(nullptr)
{
}

int
ParameterModel::rowCount(const QModelIndex& /*parent*/) const
{
	return model_ == nullptr ? 0 : model_->parameterList().size();
}

int
ParameterModel::columnCount(const QModelIndex& /*parent*/) const
{
	return NUM_COLUMNS;
}

QVariant
ParameterModel::data(const QModelIndex& index, int role) const
{
	if (model_ == nullptr || !index.isValid()) {
		return QVariant();
	}
	unsigned int row = index.row();
	if (row >= model_->parameterList().size()) {
		return QVariant();
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole) {
		switch (index.column()) {
		case 0:
			return model_->parameterList()[row].name().c_str();
			break;
		case 1:
			return model_->parameterList()[row].minimum();
			break;
		case 2:
			return model_->parameterList()[row].maximum();
			break;
		case 3:
			return model_->parameterList()[row].defaultValue();
			break;
		}
	}
	return QVariant();
}

QModelIndex
ParameterModel::index(int row, int column, const QModelIndex& /*parent*/) const
{
	return createIndex(row, column);
}

QModelIndex
ParameterModel::parent(const QModelIndex& /*index*/) const
{
	return QModelIndex();
}

Qt::ItemFlags
ParameterModel::flags(const QModelIndex& index) const
{
	if (!index.isValid()) {
		return Qt::ItemIsEnabled;
	}

	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool
ParameterModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (model_ == nullptr || !index.isValid() || role != Qt::EditRole) {
		return false;
	}
	unsigned int row = index.row();
	if (row >= model_->parameterList().size()) {
		return false;
	}

	switch (index.column()) {
	case 0:
		{
			std::string name = value.toString().toStdString();
			if (model_->findParameterName(name)) {
				qWarning("Duplicate parameter: %s", name.c_str());
				return false;
			}
			model_->parameterList()[row].setName(name);
			emit dataChanged(index, index);
			emit parameterChanged();
			return true;
		}
	case 1:
		model_->parameterList()[row].setMinimum(value.toFloat());
		emit dataChanged(index, index);
		emit parameterChanged();
		return true;
	case 2:
		model_->parameterList()[row].setMaximum(value.toFloat());
		emit dataChanged(index, index);
		emit parameterChanged();
		return true;
	case 3:
		model_->parameterList()[row].setDefaultValue(value.toFloat());
		emit dataChanged(index, index);
		emit parameterChanged();
		return true;
	default:
		return false;
	}
}

QVariant
ParameterModel::headerData(int section, Qt::Orientation orientation, int role) const
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
ParameterModel::insertRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (model_ == nullptr || count != 1) {
		return false;
	}
	if (row < 0 || static_cast<std::size_t>(row) > model_->parameterList().size()) {
		return false;
	}
	if (model_->postureList().size() > 0 || !model_->ruleList().empty()) {
		emit errorOccurred(tr("Operation not permitted. There are postures or rules in the database."));
		return false;
	}

	if (model_->findParameterName(NEW_ITEM_NAME)) {
		qWarning("Duplicate parameter: %s", NEW_ITEM_NAME);
		return false;
	}

	beginInsertRows(QModelIndex(), row, row);
	model_->parameterList().insert(
				model_->parameterList().begin() + row,
				TRMControlModel::Parameter(NEW_ITEM_NAME, 0.0, 0.0, 0.0));
	endInsertRows();

	return true;
}

bool
ParameterModel::removeRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (model_ == nullptr || count != 1) {
		return false;
	}
	if (row < 0 || static_cast<std::size_t>(row) >= model_->parameterList().size()) {
		return false;
	}
	if (model_->postureList().size() > 0 || !model_->ruleList().empty()) {
		emit errorOccurred(tr("Operation not permitted. There are postures or rules in the database."));
		return false;
	}

	beginRemoveRows(QModelIndex(), row, row);
	model_->parameterList().erase(model_->parameterList().begin() + row);
	endRemoveRows();

	return true;
}

void
ParameterModel::resetModel(TRMControlModel::Model* model)
{
	beginResetModel();
	model_ = model;
	endResetModel();
}

// Returns the new index.
QModelIndex
ParameterModel::incrementParameterRow(const QModelIndex& index)
{
	if (model_ == nullptr || !index.isValid()) {
		return index;
	}
	if (model_->postureList().size() > 0 || !model_->ruleList().empty()) {
		emit errorOccurred(tr("Operation not permitted. There are postures or rules in the database."));
		return index;
	}

	unsigned int row = index.row();
	if (row < model_->parameterList().size() - 1U) {
		std::swap(model_->parameterList()[row], model_->parameterList()[row + 1]);
		emit dataChanged(createIndex(row, 0 /* first column */), createIndex(row + 1, NUM_COLUMNS - 1 /* last column */));
		return createIndex(row + 1, NUM_COLUMNS - 1 /* last column */);
	}
	return index;
}

// Returns the new index.
QModelIndex
ParameterModel::decrementParameterRow(const QModelIndex& index)
{
	if (model_ == nullptr || !index.isValid()) {
		return index;
	}
	if (model_->postureList().size() > 0 || !model_->ruleList().empty()) {
		emit errorOccurred(tr("Operation not permitted. There are postures or rules in the database."));
		return index;
	}

	unsigned int row = index.row();
	std::swap(model_->parameterList()[row - 1], model_->parameterList()[row]);
	emit dataChanged(createIndex(row - 1, 0 /* first column */), createIndex(row, NUM_COLUMNS - 1 /* last column */));
	return createIndex(row - 1, NUM_COLUMNS - 1 /* last column */);
}

} // namespace GS
