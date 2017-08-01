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
