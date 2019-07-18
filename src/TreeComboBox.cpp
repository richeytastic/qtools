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

#include <TreeComboBox.h>
#include <QTreeView>
#include <QMouseEvent>
using QTools::TreeComboBox;


TreeComboBox::TreeComboBox( QWidget* prnt) : QComboBox(prnt), _skipNextHide(false)
{
    setView( new QTreeView(this));
    view()->viewport()->installEventFilter(this);
}   // end ctor


bool TreeComboBox::eventFilter( QObject* obj, QEvent* ev)
{
    if ( ev->type() == QEvent::MouseButtonPress && obj == view()->viewport())
    {
        QMouseEvent* mev = static_cast<QMouseEvent*>(ev);
        QModelIndex idx = view()->indexAt(mev->pos());
        if ( !view()->visualRect(idx).contains(mev->pos()))
            _skipNextHide = true;
    }   // end if
    return false;
}   // end eventFilter


void TreeComboBox::showPopup()
{
    setRootModelIndex( QModelIndex());
    QComboBox::showPopup();
}   // end showPopup


void TreeComboBox::hidePopup()
{
    setRootModelIndex(view()->currentIndex().parent());
    setCurrentIndex(view()->currentIndex().row());
    if ( _skipNextHide)
        _skipNextHide = false;
    else
        QComboBox::hidePopup();
}   // end hidePopup
