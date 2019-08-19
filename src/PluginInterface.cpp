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

#include <PluginInterface.h>
#include <iostream>
using QTools::PluginInterface;

QStringList PluginInterface::interfaceIds() const
{
    if ( _iids.isEmpty())
        return QStringList() << this->displayName();
    return _iids;
}   // end interfaceIds


PluginInterface* PluginInterface::iface( const QString& qs) const
{
    return _plugins.count(qs) > 0 ? _plugins.value(qs) : nullptr;
}   // end iface


void PluginInterface::appendPlugin( PluginInterface* plugin)
{
    _iids << plugin->displayName();
    _plugins.insert( plugin->displayName(), plugin);
}   // end appendPlugin
