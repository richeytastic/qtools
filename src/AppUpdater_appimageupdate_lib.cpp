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
#include <iostream>
using QTools::AppUpdater;

/**
 * This version of the AppUpdater is preferred but currently the
 * appimageupdate library has dependent library binary compatibilities
 * that need to be resolved.
 */
#ifdef __linux__
#include <appimage/update.h>
#endif

namespace QTools {

class AppUpdater::AppImgUpdater
{
public:
    bool checkForChanges() const
    {
#ifdef __linux__
        return _checkForChanges();
#endif
        return false;
    }   // end checkForChanges

    bool valid() const
    {
#ifdef __linux__
        return _updater;
#endif
        return false;
    }   // end valid

    bool broken() const
    {
#ifdef __linux__
        return !_appImgPath.isEmpty() && !_updater;
#endif
        return false;
    }   // end broken

    bool onHomePath() const
    {
        return _appImgPath.startsWith( QDir::homePath());
    }   // end onHomePath

    QString update( const AppUpdater *parent) const
    {
#ifdef __linux__
        return _update( parent);
#endif
        return "";
    }   // end update

private:
    QString _appImgPath;
#ifdef __linux__
    appimage::update::Updater *_updater;


    bool _checkForChanges() const
    {
        bool hasChanges = false;
        if ( _updater && !_updater->checkForChanges( hasChanges))
            _showAppImageUpdaterStatusMessages();
        return hasChanges;
    }   // end _checkForChanges


    // See documentation on update procedure at:
    // docs.appimage.org/packaging-guide/optional/updates.html#by-using-libappimageupdate
    QString _update( const AppUpdater *parent) const
    {
        if ( !_checkForChanges())
            return QObject::tr( "No update is available for this AppImage!");

        if ( !_updater->start())
            return QObject::tr( "AppImageUpdate already running or finished!");

        QString err;
        while ( err.isEmpty() && !_updater->isDone())
        {
            QThread::msleep( 200);  // Sleep for a bit
            double prog;
            if ( _updater->progress( prog))
                parent->_informProgress( 100.0 * prog);
            else
                err = QObject::tr("Couldn't update progress from AppImageUpdate!");
            _showAppImageUpdaterStatusMessages();
        }   // end while

        if ( _updater->hasError())
        {
            _showAppImageUpdaterStatusMessages();
            if ( err.isEmpty())
                err = QObject::tr( "Failed to update AppImage!");
            std::cerr << "WARNING: appimage::update::Updater failed to update AppImage!" << std::endl;
        }   // end if

        // Since the AppImage updater makes a backup with extension .zs-old, delete this file.
        QFile backupfile( _appImgPath + ".zs-old");
        if ( backupfile.exists())
        {
            const std::string fname = backupfile.fileName().toLocal8Bit().toStdString();
            std::cerr << "Removing backup " << fname << std::endl;
            if ( !backupfile.remove())
                std::cerr << "WARNING: Unable to remove " << fname << std::endl;
        }   // end if

        return err;
    }   // end _update


    bool _printAppImageUpdateInfo() const
    {
        bool valid = true;
        try
        {
            const std::string uinfo = _updater->updateInformation();
            if ( uinfo.empty())
                std::cerr << "WARNING: AppImage has no info about how to retrieve updates!" << std::endl;
            else
                std::cerr << uinfo << std::endl;
        }   // end try
        catch ( const std::runtime_error&)
        {
            std::cerr << "WARNING: AppImage could not be parsed!" << std::endl;
            valid = false;
        }   // end catch
        return valid;
    }   // end printAppImageUpdateInfo


    void _showAppImageUpdaterStatusMessages() const
    {
        std::string msg;
        while ( _updater->nextStatusMessage( msg))
            std::cerr << "[appimage::update::Updater]: " << msg << std::endl;
    }   // end _showAppImageUpdaterStatusMessages


    void _setBroken()
    {
        if ( _updater)
        {
            delete _updater;
            _updater = nullptr;
        }   // end if
    }   // end _setBroken

public:
    AppImgUpdater() : _updater(nullptr)
    {
        const QString appExe = QFileInfo(QCoreApplication::applicationFilePath()).canonicalFilePath();
        QString appImg = qEnvironmentVariable("_");
        // On Linux, recording the information below will give the location
        // of the AppImage if the application is in that format while
        // QCoreApplication::applicationFilePath() will return the exe
        // in the temporary filesystem mounted by the AppImage.
        if ( appImg[0] == ".")
            appImg = qEnvironmentVariable("PWD") + "/" + appImg;
        appImg = QFileInfo(appImg).canonicalFilePath();
        if ( appExe != appImg)
        {
            _appImgPath = appImg;
            _updater = new appimage::update::Updater( appImg.toLocal8Bit().toStdString(), true/*overwrite*/);
            std::string desc;
            if ( _updater->describeAppImage( desc))
            {
                std::cerr << desc << std::endl;
                if ( !_printAppImageUpdateInfo())
                    _setBroken();
            }   // end if
            else
            {
                std::cerr << "WARNING: Unable to get description from AppImage!" << std::endl;
                _showAppImageUpdaterStatusMessages();
                _setBroken();
            }   // end else
        }   // end if
    }   // end ctor

    ~AppImgUpdater() { _setBroken();}
#endif
};  // end class
}   // end namespace


AppUpdater::AppUpdater() : _appImgUpdater( new AppUpdater::AppImgUpdater)
{
    _appExe = QFileInfo(QCoreApplication::applicationFilePath()).canonicalFilePath();
}   // end ctor


AppUpdater::~AppUpdater() { delete _appImgUpdater;}


bool AppUpdater::isAppImage( bool *updav) const
{
    if ( updav)
        *updav = _appImgUpdater->checkForChanges();
    return _appImgUpdater->valid();
}   // end isAppImage


void AppUpdater::setFiles( const QStringList &fns) { _fpaths = fns;}
void AppUpdater::setAppTargetDir( const QString &rp) { _relPath = rp;}
void AppUpdater::setDeleteDir( const QString &oldRoot) { _oldRoot = oldRoot;}


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

}   // end namespace


bool AppUpdater::isPrivileged() const
{
    bool ok = false;
    if ( _appImgUpdater->valid())
        ok = _appImgUpdater->onHomePath();
    else
        ok = _appExe.startsWith( QDir::homePath());
    return ok || isRoot();
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
        if ( _appImgUpdater->valid())
            _err = _appImgUpdater->update( this);
        else if ( _appImgUpdater->broken())
            _err = tr("Cannot update broken/invalid AppImage!");
        else
            _updateFiles();
    }   // end if
    emit onFinished( _err);
}   // end run


namespace {

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
