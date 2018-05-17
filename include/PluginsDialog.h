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

#ifndef QTOOLS_PLUGINS_DIALOG_H
#define QTOOLS_PLUGINS_DIALOG_H

#include <QDialog>
#include "PluginInterface.h"
#include "PluginsLoader.h"

namespace Ui { class PluginsDialog; }

namespace QTools {

class QTools_EXPORT PluginsDialog : public QDialog
{ Q_OBJECT
public:
    explicit PluginsDialog(QWidget *parent = 0);
    virtual ~PluginsDialog();

    // Add plugins to the tree
    void addPlugins( const PluginsLoader&);

private:
    Ui::PluginsDialog *ui;
    PluginsDialog( const PluginsDialog&);   // NO COPY
    void operator=( const PluginsDialog&);  // NO COPY
};  // end class

}   // end namespace

#endif

