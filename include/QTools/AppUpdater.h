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
#include <QFileDevice>
#include <QThread>

namespace QTools {

class AppUpdater : public QThread
{ Q_OBJECT
public:
    // Record the path to the application exe. This should be done as
    // early as possible in the execution of the application since its
    // possible later code may change the PWD environment variable (on Linux).
    // This function is used to record the command line executed application
    // name (as opposed to the current file deemed the executable). This is
    // so that we can determine later if this is being run from an AppImage.
    static bool recordAppExe();

    // Set the path to the appimagetool used for repackaging updated AppImage.
    // Returns true iff the given path is to an existing executable file.
    static bool setAppImageToolPath( const QString &filepath);

    // Returns true iff the running app is an AppImage.
    static bool isAppImage();

    // Provide a list of update/patch archive files. Files in archives
    // later in the list that are in earlier archives are ignored.
    AppUpdater( const QStringList &files, const QString &relpath, const QString &backStore);

signals:
    void onStartedExtraction();
    void onStartedMovingFiles();
    void onStartedRepackaging();    // Only emitted for AppImage versions.
    void onFinished( const QString&);

private:
    void run() override;

    bool _extractFiles();
    void _repackageApp();

    const QStringList _fpaths;
    const QString _relPath;
    const QString _oldRoot;
    QString _err;

    static QString s_appImageTool;
    static QString s_appExe;
    static QFileDevice::Permissions s_appExePermissions;
};  // end class

}   // end namespace
     
#endif