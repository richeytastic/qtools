/************************************************************************
 * Copyright (C) 2020 Richard Palmer
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

#include <PluginsDialog.h>
#include <PluginInterface.h>
#include <ui_PluginsDialog.h>
using QTools::PluginsDialog;
using QTools::PluginsLoader;

PluginsDialog::PluginsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::PluginsDialog)
{
    ui->setupUi(this);
    connect( ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
}   // end ctor


PluginsDialog::~PluginsDialog() { delete ui;}   // end dtor


void PluginsDialog::addPlugins( const PluginsLoader& ploader)
{
    const QList<PluginsLoader::PluginMeta>& plugins = ploader.plugins();

    for ( const PluginsLoader::PluginMeta& pmeta : plugins)
    {
        QTreeWidgetItem *pluginItem = new QTreeWidgetItem(ui->treeWidget);
        pluginItem->setText(0, pmeta.filepath);
        QFont font = pluginItem->font(0);
        font.setBold(true);
        pluginItem->setFont(0, font);

        if ( !pmeta.plugin)   // Show plugin in red italics if it couldn't be loaded
        {
            //pluginItem->setTextColor(0, QColor::fromRgbF(1,0,0));
            pluginItem->setForeground(0, QColor::fromRgbF(1,0,0));
            QFont font = pluginItem->font(0);
            font.setItalic(true);
            pluginItem->setFont(0, font);
        }   // end if
        else
        {
            ui->treeWidget->setItemExpanded(pluginItem, true);
            // TODO Add in user selected enabling/disabling of dynamic plugins (requires restart).
            //pluginItem->setCheckState(0, Qt::CheckState::Checked);
            // Get the names of the available interfaces in this plugin
            const QStringList pnames = pmeta.plugin->interfaceIds();
            for ( const QString& pname : pnames)
            {
                const QTools::PluginInterface* iface = pmeta.plugin->iface(pname);
                if (iface)
                {
                    QTreeWidgetItem *iitem = new QTreeWidgetItem(pluginItem);
                    const QString cname = iface->metaObject()->className();
                    iitem->setText(0, iface->displayName() + " (" + cname + ")");
                    iitem->setIcon(0, *iface->icon());
                    QFont font = iitem->font(0);
                    font.setItalic(true);
                    iitem->setFont(0, font);
                }   // end if
            }   // end for
        }   // end if
    }   // end for
}   // end addPlugins
