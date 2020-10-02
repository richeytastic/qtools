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

class AppUpdater : public QThread
{ Q_OBJECT
public:
    // Upon construction, records the application exe's path. Since later code
    // may change the current working directory, this object should be instatiated
    // as early as possible. By recording the application exe's path and comparing
    // it on Linux to the execution environment we can tell if this application
    // is an app image.
    AppUpdater();
    ~AppUpdater() override;

    // Returns true if the user is allowed to update the app given their privileges.
    bool isPrivileged() const;

    // Returns true iff the running app is an AppImage.
    bool isAppImage() const;

    // Provide a list of update/patch archive files. Files in archives
    // later in the list that are in earlier archives are ignored.
    // Not necessary if the app is in AppImage format.
    void setFiles( const QStringList &files);

    // Set the directory to be updating specified as relative to
    // the application's directory path.
    // Not necessary if the app is in AppImage format.
    void setAppTargetDir( const QString &relpath);

    // Specify the location where the original versions of files that
    // are being updated are moved to in the filesystem.
    // Not necessary if the app is in AppImage format.
    void setDeleteDir( const QString &backStore);

signals:
    void onAppImageUpdatePercent( double) const;
    void onFinished( const QString&) const;

private:
    void run() override;

    bool _updateFiles();
    void _updateAppImage();
    void _informProgress( double) const;

    QString _appExe;
    class AppImgUpdater;
    AppImgUpdater *_appImgUpdater;

    QStringList _fpaths;
    QString _relPath;
    QString _oldRoot;
    QString _err;
};  // end class

}   // end namespace
     
#endif
