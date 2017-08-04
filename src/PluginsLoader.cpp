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

#include "PluginsLoader.h"
#include "PluginInterface.h"
using QTools::PluginsLoader;
using QTools::PluginInterface;
#include <QPluginLoader>
#include <QApplication>
#include <iostream>


QDir findPluginsDir()
{
    // Set the plugins directory for the non-static plugins
    QDir pluginsDir = QDir(qApp->applicationDirPath());
    /*
#if defined(Q_OS_WIN)
    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
        pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS")
    {
        pluginsDir.cdUp();
        pluginsDir.cdUp();
        pluginsDir.cdUp();
    }   // end if
#endif
    */
    pluginsDir.cd("plugins");
    //const QString pluginsPath = pluginsDir.absolutePath();
    return pluginsDir;
}   // end findPluginsDir


PluginsLoader::PluginsLoader()
{
    _pluginsDir = findPluginsDir();
    _pluginFileNames = _pluginsDir.entryList(QDir::Files);
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
            emit onLoadedPlugin( NULL, cname);
            continue;
        }   // end if
        numLoaded++;
        emit onLoadedPlugin( pluginInterface);
        emit onLoadedPlugin( pluginInterface, cname);
    }   // end foreach

    // Load the dynamic plugins and store their file names
    foreach ( QString fileName, _pluginFileNames)
    {
        const QString filePath = _pluginsDir.absoluteFilePath(fileName);
        QPluginLoader loader( filePath);
        QObject *plugin = loader.instance();
        if ( !plugin)
        {
            std::cerr << "[ERROR] QTools::PluginsLoader::loadPlugins: Qt dynamically loaded plugin is not a QObject (Qt issue?)!" << std::endl;
            std::cerr << "Tried to load plugin: " << filePath.toStdString();
            emit onLoadedPlugin( NULL, fileName);
            continue;
        }   // end if

        QTools::PluginInterface* pluginInterface = qobject_cast<QTools::PluginInterface*>( plugin);
        if ( !pluginInterface)
        {
            std::cerr << "[ERROR] QTools::PluginsLoader::loadPlugins: "
                << "Qt dynamically loaded plugin does not implement QTools::PluginInterface so skipping it!" << std::endl;
            std::cerr << "Tried to load plugin: " << fileName.toStdString();
            emit onLoadedPlugin( NULL, fileName);
            continue;
        }   // end if

        numLoaded++;
        emit onLoadedPlugin( pluginInterface);
        emit onLoadedPlugin( pluginInterface, fileName);
    }   // end foreach

    return numLoaded;
}   // end loadPlugins
