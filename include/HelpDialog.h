/************************************************************************
 * Copyright (C) 2019 Richard Palmer
 *
 * Cliniface is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cliniface is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#ifndef QTOOLS_HELP_DIALOG_H
#define QTOOLS_HELP_DIALOG_H

#include "TreeModel.h"
#include <QDialog>
#include <QTreeView>
#include <QTextBrowser>
#include <QSplitter>

namespace QTools {

class QTools_EXPORT HelpDialog : public QDialog
{ Q_OBJECT
public:
    explicit HelpDialog( QWidget *parent = nullptr);
    ~HelpDialog() override;

    /**
     * The search path is the root of all content. Relative paths specified in HTML
     * files are with reference to this root.
     */
    void setSearchPath( const QString&);

    /**
     * Set the table of contents according to the given model which will be
     * taken ownership of and used as the QTreeView's data. Any existed model
     * is deleted if deleteExistingModel = true.
     */
    void setTableOfContents( TreeModel* toc=nullptr, bool deleteExistingModel=true);

    /**
     * Set the TextBrowser with content from the given file returning false iff the file couldn't be read.
     */
    void setContent( const QString& htmlfile);

protected:
    QSize sizeHint() const override;

private:
    QString _wprfx;
    QSplitter *_splitter;
    QTreeView *_tview;
    QTextBrowser *_tbrowser;
    class TreeView;
    void _setContent( const QString&);
    HelpDialog( const HelpDialog&) = delete;
    void operator=( const HelpDialog&) = delete;
};  // end class

}   // end namespace

#endif
