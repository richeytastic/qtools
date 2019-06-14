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

#ifndef QTOOLS_PLUGIN_INTERFACE_H
#define QTOOLS_PLUGIN_INTERFACE_H

/**
 * Standard plugin interface for use with QTools::PluginsPool.
 * This interface allows for several different class definitions
 * that implement this interface, all hidden within a single source
 * file and exposed through this single header by the use of
 * different display names. Clients should call setCurrentInterface
 * with the name of the interface before using the other functions
 * (getIcon, getDisplayName, and any others defined in child classes)
 * so that the correct functionality is obtained.
 *
 * Note that this interface needs to inherit from QObject so that plugins
 * can be checked as instantiations of this interface using qobject_cast.
 */

#include <QStringList>
#include <QKeySequence>
#include <QString>
#include <QIcon>
#include <QtPlugin>
#include "QTools_Export.h"

namespace QTools {

// The interface to interact with loaded plugins.
class QTools_EXPORT PluginInterface : public QObject
{ Q_OBJECT
public:
    virtual ~PluginInterface(){}

    // If this PluginInterface object is a collection of different interfaces,
    // get the list of unique interface ids that are accessible through this collection.
    // If class derived from this interface definition only defines and implements a single
    // object, the returned list will have only a single member: what's returned from this
    // instance's implementation of getDisplayName().
    virtual QStringList interfaceIds() const;

    // Gets the plugin interface. Allows collection classes.
    // Returns this object if it expresses the correct interface, or the interface for
    // the specified class. The returned object should be cast to the derived class.
    // Plugins are not required to implement this function.
    virtual PluginInterface* iface( const QString&) const { return nullptr;}

    // Returns the icon for this plugin.
    // Plugins are not required to implement this function.
    virtual const QIcon* icon() const { return nullptr;}

    // Returns the key shortcut for this plugin.
    // Plugins are not required to implement this function.
    virtual const QKeySequence* shortcut() const { return nullptr;}

    // The name used to display this plugin interface for menus, legend, etc.
    // Plugins are not required to implement this function but the
    // default is the name of the class.
    virtual QString displayName() const { return metaObject()->className();}

    // Define a tool tip to be displayed for this action (none by default).
    virtual QString toolTip() const { return "";}

    // Complex plugins may want to define a widget to handle configuration.
    virtual QWidget* widget() const { return nullptr;}
};  // end class

}   // end namespace

#endif
