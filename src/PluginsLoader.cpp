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

#include <PluginsLoader.h>
#include <PluginInterface.h>
using QTools::PluginsLoader;
using QTools::PluginInterface;
#include <QPluginLoader>
#include <QApplication>
#include <iostream>


PluginsLoader::PluginsLoader( const std::string& pluginsDir)
    : _pluginsDir(pluginsDir.c_str())
{
    if ( !_pluginsDir.exists() || !_pluginsDir.isReadable())
    {
        std::cerr << "[WARNING] QTools::PluginsLoader: " << pluginsDir << " either doesn't exist, or isn't readable!" << std::endl;
        std::cerr << "                                 Trying for 'plugins' directory in application root directory..." << std::endl;
        _pluginsDir = QDir(qApp->applicationDirPath());
        _pluginsDir.cd("plugins");
    }   // end if

    if ( !_pluginsDir.exists())
        std::cerr << "[WARNING] QTools::PluginsLoader: " << _pluginsDir.absolutePath().toStdString() << " doesn't exist!" << std::endl;
    else if ( !_pluginsDir.isReadable())
        std::cerr << "[WARNING] QTools::PluginsLoader: " << _pluginsDir.absolutePath().toStdString() << " can't be read from!" << std::endl;
}   // end ctor


PluginsLoader::PluginMeta::PluginMeta( const QString& fp, const PluginInterface* pi, bool ok)
    : filepath(fp), plugin(pi), loaded(ok)
{
}   // end ctor


size_t PluginsLoader::loadPlugins()
{
    size_t numLoaded = 0;
    // Load the static plugins
    foreach ( QObject *plugin, QPluginLoader::staticInstances())
    {
        QString cname = plugin->metaObject()->className();
        std::cerr << "[STATUS] QTools::PluginsLoader::loadPlugins(): Found static library " << cname.toStdString() << std::endl;
        QTools::PluginInterface* pluginInterface = qobject_cast<QTools::PluginInterface*>( plugin);
        if ( !pluginInterface)
        {
            std::cerr << "[ERROR] QTools::PluginsLoader::loadPlugins(): "
                << "Qt statically loaded plugin does not implement QTools::PluginInterface so skipping it!" << std::endl;
            std::cerr << "Tried to load plugin: " + cname.toStdString() << std::endl;
            emit loadedPlugin( NULL, cname);
            continue;
        }   // end if
        numLoaded++;
        emit loadedPlugin( pluginInterface, cname);
    }   // end foreach

    // Load the dynamic plugins and store them
    const QStringList fnames = _pluginsDir.entryList( QDir::Files | QDir::Readable, QDir::Type | QDir::Name);
    foreach ( const QString& fname, fnames)
    {
        const QString fpath = _pluginsDir.absoluteFilePath(fname);
        QPluginLoader loader( fpath);

        QObject *plugin = loader.instance();
        QTools::PluginInterface* pluginInterface = NULL;
        if ( !plugin)
            std::cerr << "[ERROR] QTools::PluginsLoader::loadPlugins: Qt dynamically loaded plugin is not a QObject (Qt issue?)!" << std::endl;
        else
        {
            pluginInterface = qobject_cast<QTools::PluginInterface*>( plugin);
            if ( !pluginInterface)
                std::cerr << "[ERROR] QTools::PluginsLoader::loadPlugins: "
                    << "Qt dynamically loaded plugin does not implement QTools::PluginInterface so skipping it!" << std::endl;
        }   // end else

        if ( !plugin || !pluginInterface)
        {
            std::cerr << "Tried to load plugin: " << fpath.toStdString();
            emit loadedPlugin( NULL, fpath);
            PluginMeta pmeta( fpath, NULL, false);
            _plugins << pmeta;
            continue;
        }   // end if

        PluginMeta pmeta( fpath, pluginInterface, true);
        _plugins << pmeta;
        numLoaded++;
        emit loadedPlugin( pluginInterface, fpath);
    }   // end foreach

    return numLoaded;
}   // end loadPlugins
