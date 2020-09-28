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

#include <AppUpdater.h>
#include <quazip5/JlCompress.h>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QProcess>
#include <iostream>
using QTools::AppUpdater;


// static definitions
QString AppUpdater::s_appImageTool;
QString AppUpdater::s_appExe;
QFileDevice::Permissions AppUpdater::s_appExePermissions;

// public static
bool AppUpdater::recordAppExe()
{
    QString appExe;
#ifdef _WIN32
    appExe = QCoreApplication::applicationFilePath();
#elif __linux__
    appExe = qEnvironmentVariable("_");
    if ( appExe[0] == ".")
        appExe = qEnvironmentVariable("PWD") + "/" + appExe;    // Path to the AppImage
#endif
    s_appExe = QFileInfo(appExe).canonicalFilePath();
    s_appExePermissions = QFile( s_appExe).permissions();
    return isAppImage();
}   // end recordAppExe


// public static
bool AppUpdater::setAppImageToolPath( const QString &fpath)
{
    s_appImageTool = "";
    const QFileInfo finfo( fpath);
    if ( finfo.exists() && finfo.isExecutable())
        s_appImageTool = fpath;
    return !s_appImageTool.isEmpty();
}   // end setAppImageToolPath


// public static
bool AppUpdater::isAppImage()
{
#ifdef __linux__
    if ( s_appExe.isEmpty())    // Record the application exe if not already done
        recordAppExe();
    // We assume this is an AppImage if the applicationFilePath does NOT match s_appExe.
    // This is because s_appExe will be the filepath to the AppImage before it mounts the
    // temporary file system from which this application is actually run.
    return QCoreApplication::applicationFilePath() != s_appExe;
#endif
    return false;
}   // end isAppImage


AppUpdater::AppUpdater( const QStringList &fns, const QString &rp, const QString &oldRoot)
    : _fpaths(fns), _relPath(rp), _oldRoot( oldRoot) {}


void AppUpdater::run()
{
    _err = QString();
    // If an old update directory exists, remove it.
    if ( QFileInfo(_oldRoot).exists() && !QDir(_oldRoot).removeRecursively())
        _err = tr("Couldn't delete \"%1\"!").arg(_oldRoot);
    else if ( _extractFiles() && isAppImage())
        _repackageApp();
    emit onFinished( _err);
}   // end run


void AppUpdater::_repackageApp()
{
    emit onStartedRepackaging();

    if ( s_appImageTool.isEmpty())
    {
        _err = tr("Unable to repackage AppImage; tool not set!");
        return;
    }   // end if

    QTemporaryFile newAppImg;
    if ( !newAppImg.open())
    {
        _err = tr("Unable to open temporary file to write new AppImage!");
        return;
    }   // end if

    QStringList args;
    // The application directory is just the location of the temporary filesystem
    args << "-n" << (QCoreApplication::applicationDirPath() + "/../..") << newAppImg.fileName();
    QProcess *appImgProc = new QProcess(this);
    appImgProc->start( s_appImageTool, args);
    if ( !appImgProc->waitForFinished( 30000))
        _err = tr("Repackaging of AppImage timed out!");
    delete appImgProc;

    if ( _err.isEmpty())
    {
        // Copy the newly repackaged AppImage to the original location.
        if ( !QFile::remove( s_appExe))
            _err = tr("Unable to remove \"%1\" to perform update").arg( s_appExe);
        else
        {
            newAppImg.setAutoRemove(false);
            if ( !newAppImg.rename( s_appExe))
                _err = tr("Failed to rename newly repackaged AppImage!");
            else
                QFile( s_appExe).setPermissions( s_appExePermissions);  // Reset permissions
        }   // end if
    }   // end if
}   // end _repackageApp


namespace {

QString now2OldThenNew2Now( const QString &oldRoot, const QString &nowRoot, const QString &newRoot)
{
    QString err;
    const QStringList dirEntries = QDir(newRoot).entryList( QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    for ( const QString &name : dirEntries)
    {
        const QString oldpath = oldRoot + "/" + name;
        const QString nowpath = nowRoot + "/" + name;
        const QString newpath = newRoot + "/" + name;  // Exists already

        if ( QFileInfo(newpath).isDir()) // Recurse if directory
            err = now2OldThenNew2Now( oldpath, nowpath, newpath);
        else
        {
            QFile newfile(newpath);
            QFile nowfile(nowpath);

            std::cerr << "Updating \"" << nowpath.toLocal8Bit().toStdString() << "\" ... ";

            // If file exists, move to within the old directory since can't delete immediately
            if ( nowfile.exists())
            {
                const QDir olddir(QFileInfo(oldpath).path());
                if ( !olddir.exists() && !olddir.mkpath(olddir.path()))
                {
                    std::cerr << "FAILED (A)!" << std::endl;
                    return QString("Couldn't create \"%1\"").arg(olddir.path());
                }   // end if

                if ( !nowfile.rename(oldpath))
                {
                    std::cerr << "FAILED (B)!" << std::endl;
                    return QString("Couldn't move \"%1\" to \"%2\"").arg(nowpath).arg(oldpath);
                }   // end if
            }   // end if

            if ( !newfile.rename(nowpath))
            {
                std::cerr << "FAILED (C)!" << std::endl;
                return QString("Couldn't move \"%1\" to \"%2\"").arg(newpath).arg(nowpath);
            }   // end if

            std::cerr << "done" << std::endl;
        }   // end else
    }   // end for
    return err;
}   // end now2OldThenNew2Now

}   // end namespace


bool AppUpdater::_extractFiles()
{
    emit onStartedExtraction();

    // Extract all files from each archive into the same temporary directory
    // in reverse order. This ensures that the newer files with the same names
    // clobber the older files with the same names.
    QTemporaryDir newRoot;
    std::cerr << "Extracting archives..." << std::endl;
    for ( int i = _fpaths.size() - 1; i >= 0; --i)
    {
        QStringList flst = JlCompress::extractDir( _fpaths.at(i), newRoot.path());
        const bool extractedOkay = flst.size() > 0;
        if ( !extractedOkay)
        {
            _err = tr("Failed to extract archive!");
            return false;
        }   // end if
    }   // end for

    emit onStartedMovingFiles();

    const QString nowRoot = QFileInfo( QCoreApplication::applicationDirPath() + "/" + _relPath).canonicalFilePath();
    _err = now2OldThenNew2Now( _oldRoot, nowRoot, newRoot.path());
    return _err.isEmpty();
}   // end _extractFiles
