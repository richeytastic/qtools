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

#ifndef QTOOLS_TREE_MODEL_H
#define QTOOLS_TREE_MODEL_H

/**
 * A read only data model.
 */

#include "TreeItem.h"    // QTools
#include <QAbstractItemModel>
#include <QString>

namespace QTools {

class QTools_EXPORT TreeModel : public QAbstractItemModel
{ Q_OBJECT
public:
    explicit TreeModel( QObject *prnt = nullptr);
    ~TreeModel() override;

    /**
     * Set a new root for this model and return it. Do NOT delete the returned
     * TreeItem; it will be deleted when the TreeModel is destroyed. New rows
     * should be added using TreeItem::appendChild on the returned root.
     */
    TreeItem* setNewRoot( const QVector<QVariant>& data);

    /**
     * Only Qt::DisplayRole is supported for now.
     */
    QVariant data( const QModelIndex &index, int role=Qt::DisplayRole) const override;

    Qt::ItemFlags flags( const QModelIndex &index) const override;
    QVariant headerData( int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index( int row, int col, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent( const QModelIndex &index) const override;
    int rowCount( const QModelIndex &parent = QModelIndex()) const override;
    int columnCount( const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Linear search of the model to find the index with the given data at the given column.
     */
    QModelIndex find( const QVariant& data, int col) const;

private:
    TreeItem *_rootItem;
    QModelIndex _find( TreeItem*, const QVariant&, int) const;
};  // end class

}   // end namespace

#endif
