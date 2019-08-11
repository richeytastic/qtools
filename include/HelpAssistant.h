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

#ifndef QTOOLS_HELP_ASSISTANT_H
#define QTOOLS_HELP_ASSISTANT_H

#include "HelpBrowser.h"
#include <QTemporaryDir>

namespace QTools {

class QTools_EXPORT HelpAssistant 
{
public:
    /**
     * Pass in the directory containing html files for content.
     * There should at least be a file called "index.html" within.
     * Upon creation, all documents in the given directory are copied
     * into a temporary working directory to which new documents can
     * be dynamically added using addDocument.
     */
    explicit HelpAssistant( const QString& hdir, QWidget* parent=nullptr);
    ~HelpAssistant();

    /**
     * Add a new subdirectory to the working directory.
     */
    bool addSubDirectory( const QString& dir);

    /**
     * Add an html file at the given location within the working directory and
     * return the identifying token for the page to be used later in calls to show.
     */
    QString addDocument( const QString& dir, const QString& htmlFile);

    /**
     * Call after all documentation added to refresh what's displayed in the dialog.
     * Pass the path of the XML file which defines the table of contents. This can
     * include references to directories added using addSubDirectory.
     */
    void refreshContents( const QString& tocXMLFile);

    /**
     * Show the specified page by reference to its token.
     * If token is empty, the index page is shown.
     */
    bool show( const QString &token="");

private:
    HelpBrowser *_dialog;
    QTemporaryDir _tdir;
    void _initTempHtmlDir( const QString&);
};  // end class

}   // end namespace

#endif
