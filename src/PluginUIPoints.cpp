/************************************************************************
 * Copyright (C) 2019 Richard Palmer
 *
 * Cliniface is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cliniface is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#include <PluginUIPoints.h>
using QTools::PluginUIPoints;


void PluginUIPoints::set( const QString& tok, QMenu* m, const QAction* lla)
{
    _menus[tok] = {m, lla};
}   // end set


void PluginUIPoints::set( const QString& tok, QToolBar* tb, const QAction* lla)
{
    _tbars[tok] = {tb, lla};
}   // end set


bool PluginUIPoints::appendToMenu( QAction* act, const QString& wat)
{
    if ( _menus.count(wat) == 0)
        return false;

    QMenu *menu = static_cast<QMenu*>(_menus.value(wat).w);
    QAction *lla = const_cast<QAction*>(_menus.value(wat).a);
    menu->insertAction( lla, act);
    return true;
}   // end appendToMenu


bool PluginUIPoints::appendToToolBar( QAction* act, const QString& wat)
{
    if ( _tbars.count(wat) == 0)
        return false;

    QToolBar *tbar = static_cast<QToolBar*>(_tbars.value(wat).w);
    QAction *lla = const_cast<QAction*>(_menus.value(wat).a);
    tbar->insertAction( lla, act);
    return true;
}   // end appendToToolBar


