/************************************************************************
 * Copyright (C) 2020 Richard Palmer
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

#ifndef QTOOLS_HELP_BROWSER_H
#define QTOOLS_HELP_BROWSER_H

#include "TreeModel.h"
#include <QMainWindow>
#include <QToolButton>
#include <QTextBrowser>
#include <QTreeView>
#include <QSplitter>
// TODO Switch to QtWebEngineView instead of QTextBrowser to use Javascript etc.
// Note this requires optional Qt addon module which under Windows needs MSVC 2017.
// Only do this after setting up MSVC 2017 compiler.
//#include <QtWebEngineWidgets/QtWebEngineWidgets>


namespace QTools {

class QTools_EXPORT HelpBrowser : public QMainWindow
{ Q_OBJECT
public:
    explicit HelpBrowser( QWidget *parent = nullptr);

    /**
     * The root of all content. Relative paths specified in HTML files are with reference to this root.
     */
    void setRootDir( const QString&);

    /**
     * Set the table of contents according to the given model which will be
     * taken ownership of and used as the QTreeView's data. Any existed model
     * is deleted if deleteExistingModel = true.
     */
    void setTableOfContents( TreeModel* toc=nullptr, bool deleteExistingModel=true);

    /**
     * Set the TextBrowser with content from the given file returning false iff the file couldn't be read.
     */
    bool setContent( const QString& htmlfile);

protected:
    QSize sizeHint() const override { return QSize( 1150, 640);}

private slots:
    void _doOnSourceChanged( const QUrl&);

private:
    QString _wprfx;
    QString _rootDir;
    QSplitter *_splitter;
    QTreeView *_tview;
    QTextBrowser *_tbrowser;
    //QToolButton *_backButton;
    //QToolButton *_fwrdButton;

    class TreeView;
    void _setContent( const QString&);
    HelpBrowser( const HelpBrowser&) = delete;
    void operator=( const HelpBrowser&) = delete;
};  // end class

}   // end namespace

#endif
