/************************************************************************
 * Copyright (C) 2019 Richard Palmer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#include <TreeModel.h>
#include <cassert>
using QTools::TreeModel;
using QTools::TreeItem;


TreeModel::TreeModel( QObject *prnt) : QAbstractItemModel(prnt), _rootItem(nullptr) {}


TreeModel::~TreeModel()
{
    if ( _rootItem)
        delete _rootItem;
}   // end dtor


TreeItem* TreeModel::setNewRoot( const QVector<QVariant>& data)
{
    if ( _rootItem)
        delete _rootItem;
    _rootItem = new TreeItem( data, nullptr);
    return _rootItem;
}   // end setNewRoot


QModelIndex TreeModel::index( int row, int col, const QModelIndex &parent) const
{
    if ( !hasIndex(row, col, parent))
        return QModelIndex();

    TreeItem *parentItem = _rootItem;
    if ( parent.isValid())
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    assert(parentItem);
    TreeItem *childItem = parentItem->child(row);
    return childItem ? createIndex(row, col, childItem) : QModelIndex();
}   // end index


QModelIndex TreeModel::parent( const QModelIndex &idx) const
{
    if (!idx.isValid())
        return QModelIndex();
    TreeItem *childItem = static_cast<TreeItem*>(idx.internalPointer());
    assert(childItem);
    TreeItem *parentItem = childItem->parent();
    if ( !parentItem || parentItem == _rootItem)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}   // end parent


int TreeModel::rowCount( const QModelIndex &parent) const
{
    if ( parent.column() > 0)
        return 0;
    TreeItem *parentItem = parent.isValid() ? static_cast<TreeItem*>(parent.internalPointer()) : _rootItem;
    assert(parentItem);
    return parentItem->childCount();
}   // end rowCount


int TreeModel::columnCount( const QModelIndex &parent) const
{
    if ( parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    return _rootItem->columnCount();
}   // end columnCount


QVariant TreeModel::data( const QModelIndex &idx, int role) const
{
    if ( !idx.isValid() || role != Qt::DisplayRole)
        return QVariant();  // Invalid by default
    return static_cast<TreeItem*>(idx.internalPointer())->data(idx.column());
}   // end data


Qt::ItemFlags TreeModel::flags( const QModelIndex &idx) const
{
    return idx.isValid() ? QAbstractItemModel::flags(idx) : Qt::NoItemFlags;
}   // end flags


QVariant TreeModel::headerData( int section, Qt::Orientation orientation, int role) const
{
    return orientation == Qt::Horizontal && role == Qt::DisplayRole ? _rootItem->data(section) : QVariant();
}   // end headerDataa


QModelIndex TreeModel::_find( TreeItem* item, const QVariant& data, int col) const
{
    if ( item->data(col) == data)
        return createIndex( item->row(), col, item);

    QModelIndex idx;
    const int n = item->childCount();
    for ( int i = 0; i < n; ++i)
    {
        idx = _find( item->child(i), data, col);
        if ( idx.isValid())
            break;
    }   // end for

    return idx;
}   // end _find


QModelIndex TreeModel::find( const QVariant& data, int col) const
{
    return _find( _rootItem, data, col);
}   // end find
