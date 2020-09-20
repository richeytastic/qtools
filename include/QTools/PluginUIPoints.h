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

#ifndef QTOOLS_PLUGIN_UI_POINTS_H
#define QTOOLS_PLUGIN_UI_POINTS_H

#include "QTools_Export.h"
#include <QMap>
#include <QMenu>
#include <QToolBar>
#include <QAction>

namespace QTools {

class QTools_EXPORT PluginUIPoints
{
public:
    /**
     * Set the menu or tool bar instances to manage. If lastLogicalAction is not null,
     * actions will be appended before this action, otherwise they will be appended
     * to the widget.
     */
    void set( const QString& tok, QMenu*, const QAction* lastLogicalAction=nullptr);
    void set( const QString& tok, QToolBar*, const QAction* lastLogicalAction=nullptr);

    /**
     * Append the given action to a menu. The string should be the tag of a previously added menu.
     **/
    bool appendToMenu( QAction* act, const QString& whereAt);

    /**
     * As above but for toolbars.
     */
    bool appendToToolBar( QAction* act, const QString& whereAt);

private:
    struct Widget
    {
        QWidget *w;
        const QAction *a;
    };  // end Widget
    QMap<QString, Widget> _menus;
    QMap<QString, Widget> _tbars;
};  // end class

}   // end namespace

#endif
