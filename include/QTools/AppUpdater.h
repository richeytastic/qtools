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

    // Check if this is AppImage format (can only be true for Linux).
    static bool isAppImage();

    AppUpdater( const QString &updateFile, const QString &backStore, const QString &relpath="");

signals:
    void onFinished( const QString&);

private:
    void run() override;

    void _doWindowsUpdate();
    void _doLinuxUpdate();

    const QString _updatePath;
    const QString _oldRoot;
    const QString _relpath;
    QString _err;

    static QString s_appExe;
    static QFileDevice::Permissions s_appExePermissions;
};  // end class

}   // end namespace
     
#endif
