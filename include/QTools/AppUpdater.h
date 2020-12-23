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

#ifndef QTOOLS_APP_UPDATER_H
#define QTOOLS_APP_UPDATER_H

#include "QTools_Export.h"
#include <QThread>

namespace QTools {

class QTools_EXPORT AppUpdater : public QThread
{ Q_OBJECT
public:
    // Upon construction, records the application exe's path. Since later code
    // may change the current working directory, this object should be instatiated
    // as early as possible. By recording the application exe's path and comparing
    // it on Linux to the execution environment we can tell if this application
    // is an app image.
    AppUpdater();

    // Set the target directory to patch files into relative to the directory where the
    // application executable is located i.e. QCoreApplication::applicationDirPath().
    void setAppPatchDir( const QString &relpath);

    // Provide the update/patch archive files - typically locations of temporary files.
    // Files in archives later in the list that are in earlier archives are ignored.
    // Optionally specify paths to files to remove (rfiles) which are given relative
    // to the application patch directory. Returns immediately and fires onFinished
    // when updating is complete.
    bool update( const QStringList &files, const QStringList &rfiles=QStringList());

signals:
    void onExtracting() const;
    void onUpdating() const;
    void onRepacking() const; // Only emitted for AppImage versions
    void onFinished( const QString&) const;

private:
    void run() override;
    bool _isAppImage() const;
    bool _extractFiles( const QString&) const;
    QString _repackAppImage( const QString&, const QString&, const QString&) const;
    void _failFinish( const char*);
    QString _appFilePath;
    QStringList _fpaths;
    QStringList _rpaths;
    QString _relPath;
    QString _err;
};  // end class

}   // end namespace
     
#endif
