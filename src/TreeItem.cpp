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

#include <TreeItem.h>
using QTools::TreeItem;


TreeItem::TreeItem( const QVector<QVariant>& data, TreeItem *prnt)
    : _data(data), _parent(nullptr)
{
    if ( prnt)
    {
        _parent = prnt;
        _parent->appendChild(this);
    }   // end prnt
}   // end ctor


TreeItem::~TreeItem()
{
    qDeleteAll(_children);
}   // end dtor


void TreeItem::appendChild( TreeItem *item)
{
    if ( item)
    {
        _children.append(item);
        _rowHash[item] = _children.size() - 1;
        item->_parent = this;
    }   // end if
}   // end appendChild


TreeItem *TreeItem::parent() { return _parent;}


TreeItem *TreeItem::child( int row)
{
    return row >= 0 && row < _children.size() ? _children.at(row) : nullptr;
}   // end child


int TreeItem::childCount() const { return _children.count();}


int TreeItem::row() const
{
    return _parent ? _parent->_children.indexOf(const_cast<TreeItem*>(this)) : 0;
}   // end row


int TreeItem::childRow( const TreeItem* item) const
{
    return _rowHash.count(item) > 0 ? _rowHash.at(item) : -1;
}   // end childRow


int TreeItem::columnCount() const { return _data.count();}


QVariant TreeItem::data( int col) const
{
    return col >= 0 && col < _data.size() ? _data.at(col) : QVariant();
}   // end data
