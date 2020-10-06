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

#ifndef QTOOLS_TREE_ITEM_H
#define QTOOLS_TREE_ITEM_H

#include "QTools_Export.h"    // QTools
#include <unordered_map>
#include <QVector>
#include <QVariant>

namespace QTools {

class QTools_EXPORT TreeItem
{
public:
    // This newly constructed item will be added as a child of the parent if not null.
    explicit TreeItem( const QVector<QVariant> &data, TreeItem *parent=nullptr);
    ~TreeItem();

    void appendChild( TreeItem*);

    // Return this item's parent.
    TreeItem *parent();

    // Return the child at the given row (or nullptr if none).
    TreeItem *child( int row);

    // Return the row the given item is on or -1 if not found (hash lookup).
    int childRow( const TreeItem*) const;

    // Return the row this item is on with respect to this item's parent.
    int row() const;

    // The number of children items that this item is a parent of.
    int childCount() const;

    // The number of data elements held by this item.
    int columnCount() const;

    // The data element at the given column.
    QVariant data( int col) const;

private:
    QVector<QVariant> _data;
    TreeItem *_parent;
    QVector<TreeItem*> _children;
    std::unordered_map<const TreeItem*, int> _rowHash;
};  // end class

}   // end namespace

#endif
