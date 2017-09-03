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

#include <PluginsDialog.h>
#include <ui_PluginsDialog.h>
using QTools::PluginsDialog;


class PluginsDialog::Deleter
{ public:
    void operator()( PluginsDialog* d) { delete d;}
};  // end class

PluginsDialog::Ptr PluginsDialog::s_singleton;

// public static
PluginsDialog::Ptr PluginsDialog::get()
{
    if ( !s_singleton)
        s_singleton = Ptr( new PluginsDialog(), Deleter());
    return s_singleton;
}   // end get

// private
PluginsDialog::PluginsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::PluginsDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
    connect( ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
}   // end ctor

// private
PluginsDialog::~PluginsDialog() { delete ui;}   // end dtor


// public
void PluginsDialog::setPluginsLoader( QTools::PluginsLoader *ploader)
{
    connect( ploader, SIGNAL( onLoadedPlugin( QTools::PluginInterface*, QString)),
                this, SLOT( doOnLoadedPlugin( QTools::PluginInterface*, QString)));
}   // end setPluginsLoader


// private slot
void PluginsDialog::doOnLoadedPlugin( QTools::PluginInterface* plugin, QString text)
{
    QTreeWidgetItem *pluginItem = new QTreeWidgetItem(ui->treeWidget);
    pluginItem->setText(0, text);
    QFont font = pluginItem->font(0);
    font.setBold(true);
    pluginItem->setFont(0, font);

    if ( !plugin)   // Show plugin in red italics if it couldn't be loaded
    {
        pluginItem->setTextColor(0, QColor::fromRgbF(1,0,0));
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
        const QStringList pnames = plugin->getInterfaceIds();
        foreach ( const QString& pname, pnames)
        {
            const QTools::PluginInterface* iface = plugin->getInterface(pname);
            if (iface)
            {
                QTreeWidgetItem *iitem = new QTreeWidgetItem(pluginItem);
                const QString cname = iface->metaObject()->className();
                iitem->setText(0, iface->getDisplayName() + " (" + cname + ")");
                iitem->setIcon(0, *iface->getIcon());
                QFont font = iitem->font(0);
                font.setItalic(true);
                iitem->setFont(0, font);
            }   // end if
        }   // end foreach
    }   // end if
}   // end doOnLoadedPlugin
