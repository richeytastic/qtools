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
#include <QTemporaryDir>
#include <QProcess>
#include <iostream>
using QTools::AppUpdater;
#define APPIMAGEUPDATETOOL "appimageupdatetool-x86_64.AppImage"

namespace {
#ifdef _WIN32
#include <Windows.h>
BOOL isRunningAsAdmin()
{
    BOOL fIsRunAsAdmin = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = nullptr;

    // Allocate and initialize a SID of the administrators group.
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(
        &NtAuthority, 
        2, 
        SECURITY_BUILTIN_DOMAIN_RID, 
        DOMAIN_ALIAS_RID_ADMINS, 
        0, 0, 0, 0, 0, 0, 
        &pAdministratorsGroup))
    {
        dwError = GetLastError();
        goto Cleanup;
    }   // end if

    // Determine whether the SID of administrators group is enabled in 
    // the primary access token of the process.
    if (!CheckTokenMembership( nullptr, pAdministratorsGroup, &fIsRunAsAdmin))
    {
        dwError = GetLastError();
        goto Cleanup;
    }   // end if

Cleanup:
    // Centralized cleanup for all allocated resources.
    if (pAdministratorsGroup)
    {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = nullptr;
    }   // end if

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS != dwError)
        throw dwError;

    return fIsRunAsAdmin;
}   // end isRunningAsAdmin
#endif


bool isRoot()
{
    bool isr = false;
#ifdef _WIN32
    isr = isRunningAsAdmin();
#elif __linux__
    isr = getuid() != geteuid();
#endif
    return isr;
}   // end isRoot


QString now2OldThenNew2Now( const QString &oldRoot, const QString &nowRoot, const QString &newRoot)
{
    QString err;
    const QStringList entries = QDir(newRoot).entryList( QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    for ( const QString &name : entries)
    {
        const QString oldpath = oldRoot + "/" + name;
        const QString nowpath = nowRoot + "/" + name;
        const QString newpath = newRoot + "/" + name;  // Exists (the temp location of extracted file)

        if ( QFileInfo(newpath).isDir()) // Recurse if directory
        {
            QDir().mkpath(nowpath); // Ensuring it exists in the current path
            err = now2OldThenNew2Now( oldpath, nowpath, newpath);
        }   // end if
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


namespace QTools {

#ifndef __linux__
class AppUpdater::AppImgUpdater
{
public:
    bool isAppImage() const { return false;}
    bool onHomePath() const { return false;}
    QString update() const { return "";}
};  // end class
#else
class AppUpdater::AppImgUpdater
{
public:
    AppImgUpdater()
    {
        QString appImg = qEnvironmentVariable("_");
        // On Linux, recording the information below will give the location
        // of the AppImage if the application is in that format while
        // QCoreApplication::applicationFilePath() will return the exe
        // in the temporary filesystem mounted by the AppImage.
        if ( appImg[0] == ".")
            appImg = qEnvironmentVariable("PWD") + "/" + appImg;
        _appImgPath = QFileInfo(appImg).canonicalFilePath();
    }   // end ctor

    const QString &appImagePath() const { return _appImgPath;}

    QString update( AppUpdater *parent) const
    {
        QStringList args;
        args << "-O" << "-r" << _appImgPath;
        QProcess *uproc = new QProcess(parent);
        uproc->start( APPIMAGEUPDATETOOL, args);
        int pcnt = 0;
        while ( uproc->state() != QProcess::NotRunning)
        {
            parent->_informProgress( pcnt);
            pcnt = std::min( pcnt+1, 99);
            uproc->waitForFinished( 200);
        }   // end while
        parent->_informProgress( 100.0);
        QString err;
        if ( uproc->exitStatus() != QProcess::NormalExit || uproc->exitCode() != 0)
            err = QObject::tr("Unable to finish updating with appimageupdatetool!");
        delete uproc;
        return err;
    }   // end update

private:
    QString _appImgPath;
};  // end class
#endif
}   // end namespace


AppUpdater::AppUpdater() : _appImgUpdater( new AppUpdater::AppImgUpdater)
{
    _appExe = QFileInfo(QCoreApplication::applicationFilePath()).canonicalFilePath();
}   // end ctor


AppUpdater::~AppUpdater() { delete _appImgUpdater;}


bool AppUpdater::isAppImage() const { return _appImgUpdater->appImagePath() != _appExe;}


void AppUpdater::setFiles( const QStringList &fns) { _fpaths = fns;}
void AppUpdater::setAppTargetDir( const QString &rp) { _relPath = rp;}
void AppUpdater::setDeleteDir( const QString &oldRoot) { _oldRoot = oldRoot;}


bool AppUpdater::isPrivileged() const
{
    const QString &app = isAppImage() ? _appImgUpdater->appImagePath() : _appExe;
    return app.startsWith( QDir::homePath()) || isRoot();
}   // end isPrivileged


void AppUpdater::_informProgress( double pcnt) const
{
    emit onAppImageUpdatePercent( pcnt);
}   // end _informProgress


void AppUpdater::run()
{
    _err = QString();
    // Always remove old update directories if they exist.
    if ( QFileInfo(_oldRoot).exists() && !QDir(_oldRoot).removeRecursively())
        _err = tr("Couldn't delete \"%1\"!").arg(_oldRoot);
    if ( _err.isEmpty())
    {
        if ( isAppImage())
            _err = _appImgUpdater->update( this);
        else
            _updateFiles();
    }   // end if
    emit onFinished( _err);
}   // end run


bool AppUpdater::_updateFiles()
{
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
            std::cerr << "Failed to extract archive!" << std::endl;
            _err = tr("Failed to extract archive!");
            return false;
        }   // end if
    }   // end for

    std::cerr << "Moving files..." << std::endl;
    const QString nowRoot = QFileInfo( QCoreApplication::applicationDirPath() + "/" + _relPath).canonicalFilePath();
    _err = now2OldThenNew2Now( _oldRoot, nowRoot, newRoot.path());
    return _err.isEmpty();
}   // end _updateFiles

