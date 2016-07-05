#include "PluginsPool.h"
using QTools::PluginsPool;
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
    const QString pluginsPath = pluginsDir.absolutePath();
    std::cout << "Plugins path: " << pluginsPath.toStdString() << std::endl;
    return pluginsDir;
}   // end findPluginsDir


PluginsPool::PluginsPool()
{
    _pluginsDir = findPluginsDir();
    _pluginFileNames = _pluginsDir.entryList(QDir::Files);
}   // end ctor


size_t PluginsPool::loadPlugins()
{
    size_t numLoaded = 0;
    // Load the static plugins
    foreach ( QObject *plugin, QPluginLoader::staticInstances())
    {
        QString cname = plugin->metaObject()->className();
        QTools::PluginInterface* pluginInterface = qobject_cast<QTools::PluginInterface*>( plugin);
        if ( !pluginInterface)
        {
            std::cerr << "[ERROR] QTools::PluginsPool::loadPlugins: "
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
            std::cerr << "[ERROR] QTools::PluginsPool::loadPlugins: Qt dynamically loaded plugin is not a QObject (Qt issue?)!" << std::endl;
            std::cerr << "Tried to load plugin: " << filePath.toStdString();
            emit onLoadedPlugin( NULL, fileName);
            continue;
        }   // end if

        QTools::PluginInterface* pluginInterface = qobject_cast<QTools::PluginInterface*>( plugin);
        if ( !pluginInterface)
        {
            std::cerr << "[ERROR] QTools::PluginsPool::loadPlugins: "
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
