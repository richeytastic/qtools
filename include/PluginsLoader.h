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

#ifndef QTOOLS_PLUGINS_LOADER_H
#define QTOOLS_PLUGINS_LOADER_H

#include "QTools_Export.h"
#include <QList>
#include <QDir>

namespace QTools {

class PluginInterface;

class QTools_EXPORT PluginsLoader : public QObject
{ Q_OBJECT
public:
    /**
     * Searches for shared plugins in the given absolute directory path.
     * If not given, or path doesn't resolve to a readable directory, searches
     * for the 'plugins' directory in the application root directory.
     */
    explicit PluginsLoader( const std::string& sharedPluginsDir="");
    virtual ~PluginsLoader(){}

    /**
     * Loads all plugins returning the number of static libraries found plus the number of shared
     * plugins loaded. Only shared libraries with matching appcode are loaded; shared libraries
     * with non-matching appcodes are ignored. The appcode parameter is ignored for statically
     * linked libraries. Emits loadedPlugin for each new plugin loaded (static or shared).
     */
    size_t loadPlugins( const std::string& appcode);

    /**
     * Returns the path to the directory where shared libraries are loaded from.
     */
    const QDir& pluginsDir() const { return _pluginsDir;}

    struct PluginMeta
    {
        PluginMeta( const QString&, const PluginInterface*);
        QString filepath;               /* Absolute path to the plugin discovered */
        const PluginInterface* plugin;  /* The plugin itself (null if not loaded) */
    };  // end struct

    /**
     * Returns the list of loaded plugins.
     */
    const QList<PluginMeta>& plugins() const { return _plugins;}

signals:
    // Signal discovery of statically linked libraries emitting the class name,
    // or signal loading of dynamically linked libraries with absolute filepath.
    void loadedPlugin( PluginInterface*, const QString&);

private:
    std::string _appcode;
    QDir _pluginsDir;
    QList<PluginMeta> _plugins;

    PluginsLoader( const PluginsLoader&) = delete;
    void operator=( const PluginsLoader&) = delete;
};  // end class

}   // end namespace

#endif
