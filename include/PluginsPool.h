#ifndef QTOOLS_PLUGINS_POOL_H__
#define QTOOLS_PLUGINS_POOL_H__

#include <QDir>
#include "PluginInterface.h"
#include "QTools_Export.h"

namespace QTools
{

class QTools_EXPORT PluginsPool : public QObject
{ Q_OBJECT
public:
    PluginsPool();  // Finds all plugins in plugins directory - does not load them!
    virtual ~PluginsPool(){}

    // Loads all plugins (static + dynamic) returns number loaded.
    // Fires onLoadedPlugin for each new plugin loaded.
    size_t loadPlugins();

    const QDir& getPluginsDir() const { return _pluginsDir;}
    const QStringList& getDynamicPluginFileNames() const { return _pluginFileNames;}

signals:
    void onLoadedPlugin( QTools::PluginInterface*);
    void onLoadedPlugin( QTools::PluginInterface*, QString);

private:
    QDir _pluginsDir;
    QStringList _pluginFileNames;
};  // end class

}   // end namespace

#endif
