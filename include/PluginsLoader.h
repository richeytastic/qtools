/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

#ifndef QTOOLS_PLUGINS_LOADER_H
#define QTOOLS_PLUGINS_LOADER_H

#include <QDir>
#include "QTools_Export.h"

namespace QTools
{

class PluginInterface;

class QTools_EXPORT PluginsLoader : public QObject
{ Q_OBJECT
public:
    PluginsLoader();  // Finds all plugins in plugins directory - does not load them!
    virtual ~PluginsLoader(){}

    // Loads all plugins (static + dynamic) returns number loaded.
    // Fires onLoadedPlugin for each new plugin loaded.
    size_t loadPlugins();

    const QDir& getPluginsDir() const { return _pluginsDir;}    // The dynamic library location
    const QStringList& getDynamicPluginFileNames() const { return _pluginFileNames;}    // Dynamic libraries

signals:
    void onLoadedPlugin( QTools::PluginInterface*);
    void onLoadedPlugin( QTools::PluginInterface*, QString);

private:
    QDir _pluginsDir;
    QStringList _pluginFileNames;
};  // end class

}   // end namespace

#endif
