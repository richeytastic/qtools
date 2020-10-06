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

// To check file permissions on Windows
#ifdef _WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

// static definitions
QString AppUpdater::s_appImageTool = "";
QString AppUpdater::s_fileMoveTool = "";
void AppUpdater::setAppImageTool( const QString &v) { s_appImageTool = v;}
void AppUpdater::setFileMoveTool( const QString &v) { s_fileMoveTool = v;}


namespace {
/*
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
*/


QString toolPath( const QString &atool)
{
    const QFileInfo file(atool);
    return file.exists( atool) && file.isExecutable() ? file.canonicalFilePath() : "";
}   // end toolPath


bool moveFilesAsRoot( const QString &tool, const QString &src, const QString &dst, const QString &bck)
{
    QString program = toolPath(tool);
    if ( program.isEmpty())
        return false;

    QStringList args;
#ifdef __linux__
    args << program << src << dst << bck;
    program = "pkexec";
#elif _WIN32
    args << "-Command" << "Start-Process"
         << QString("'%1'").arg(program)
         << QString("'\"%1\" \"%2\" \"%3\"'").arg(src).arg(dst).arg(bck)
         << "-Verb" << "runAs";
    program = "powershell";
#endif
    return QProcess::execute( program, args) == 0;
}   // end moveFilesAsRoot


bool repackAppImage( const QString &tool, const QString &appDir, const QString &outAppImage)
{
    const QString appImgTool = toolPath(tool);
    if ( appImgTool.isEmpty())
        return false;

    QTemporaryDir tdir;
    if ( !tdir.isValid())
        return false;

    const QString repackfile = tdir.filePath("repacked.AppImage");

    QStringList args;
    args << "-n" << appDir << repackfile;
    if ( QProcess::execute( appImgTool, args) != 0)
        return false;

    return QFile( repackfile).rename( outAppImage);
}   // end repackAppImage
}   // end namespace


AppUpdater::AppUpdater()
{
    _appFilePath = QCoreApplication::applicationFilePath();
    // On Linux, recording the information below will give the location
    // of the AppImage if the application is in that format while
    // QCoreApplication::applicationFilePath() returns the exe
    // in the temporary filesystem mounted by the AppImage.
    // This gives a method to check if the application is AppImage.
#ifdef __linux__
    QString appImg = qEnvironmentVariable("_");
    if ( appImg[0] == ".")
        appImg = qEnvironmentVariable("PWD") + "/" + appImg;
    _appFilePath = QFileInfo(appImg).canonicalFilePath();
    //std::cerr << "AppFilePath: " << _appFilePath.toLocal8Bit().toStdString() << std::endl;
#endif
}   // end ctor


bool AppUpdater::_isAppImage() const
{
#ifdef _WIN32
    return false;
#endif
    return QCoreApplication::applicationFilePath() != _appFilePath;
}   // end _isAppImage


void AppUpdater::setAppPatchDir( const QString &rp) { _relPath = rp;}


bool AppUpdater::update( const QStringList &fns)
{
    if ( _isAppImage() && toolPath(s_appImageTool).isEmpty())
    {
        _err = tr("Missing AppImageTool!");
        return false;
    }   // end if

    _fpaths = fns;
    if ( _fpaths.isEmpty())
    {
        _err = tr("No update files provided!");
        return false;
    }   // end if

    start();
    return true;
}   // end update


void AppUpdater::run()
{
    emit onExtracting();
    QTemporaryDir extractDir;
    if ( !extractDir.isValid())
        return _failFinish( "Unable to create temporary directory for extraction!");

    const QString extractPath = extractDir.path();
    std::cerr << "Extracting archives..." << std::endl;
    if ( !_extractFiles( extractPath))
        return _failFinish( "Failed to extract archive!");

    emit onUpdating();

    // If this is an AppImage the application files are mounted as a read-only filesystem
    // so we have to copy everything to a new location to update there before repackaging.
    QString appDir = QCoreApplication::applicationDirPath();

    QTemporaryDir *newAppDir = nullptr;
    QString newAppDirPath;
    if ( _isAppImage())
    {
        newAppDir = new QTemporaryDir;
        if ( !newAppDir->isValid())
        {
            delete newAppDir;
            return _failFinish( "Unable to create temporary directory for new AppDir!");
        }   // end if

        newAppDirPath = newAppDir->path();

        const QString appImgDir = QDir( appDir + "/../..").canonicalPath();
        std::cerr << "Copying " << appImgDir.toStdString() << " to " << newAppDirPath.toStdString() << std::endl;
        if ( QProcess::execute( "cp", {"-r", appImgDir, newAppDirPath}) != 0)
        {
            delete newAppDir;
            return _failFinish( "Failed to copy app dir to temp dir!");
        }   // end if
        appDir = QDir( newAppDirPath + "/usr/bin").canonicalPath();
        std::cerr << "New binary directory: " << appDir.toLocal8Bit().toStdString() << std::endl;
    }   // end if

    const QString patchDir = QDir( appDir + "/" + _relPath).canonicalPath();

    static const QString appName = QCoreApplication::applicationName();
    static const QString backupPath = QDir::tempPath() + QString("/%1_update_backup").arg(appName);
    QDir( backupPath).removeRecursively();  // Remove this directory if present

    std::cerr << "Patching directory: " << patchDir.toLocal8Bit().toStdString() << std::endl;

    // Write into the application patch directory if we have permission.
    // Otherise invoke via detached process to allow OS to request permission.
    if ( QFile::permissions( patchDir) & QFileDevice::WriteUser)
    {
        if ( !moveFiles( extractPath, patchDir, backupPath))
        {
            moveFiles( backupPath, patchDir, extractPath);   // Restore
            return _failFinish( "Failed to update files!");
        }   // end if
    }   // end if
    else
    {
        std::cerr << "No permissions - invoking " << s_fileMoveTool.toLocal8Bit().toStdString() << std::endl;
        if ( !moveFilesAsRoot( s_fileMoveTool, extractPath, patchDir, backupPath))
            return _failFinish( "Failed to update files in separate process!");
    }   // end else

    emit onRepacking();

    // Repackage the updated application directory as an AppImage?
    if ( _isAppImage())
    {
        std::cerr << "Repacking AppImage..." << std::endl;
        if ( !repackAppImage( s_appImageTool, newAppDirPath, _appFilePath))
        {
            delete newAppDir;
            return _failFinish( "Failed to repack AppImage!");
        }   // end if
        delete newAppDir;
    }   // end if

    std::cerr << "Finished" << std::endl;

    emit onFinished( _err);
}   // end run


void AppUpdater::_failFinish( const char *err)
{
    _err = tr(err);
    emit onFinished(err);
}   // end _failFinish


bool AppUpdater::_extractFiles( const QString &extractDir) const
{
    // Extract all files from each archive into the same temporary directory
    // in reverse order. This ensures that the newer files with the same names
    // clobber the older files with the same names.
    for ( int i = _fpaths.size() - 1; i >= 0; --i)
    {
        const QStringList flst = JlCompress::extractDir( _fpaths.at(i), extractDir);
        if ( flst.size() == 0)
            return false;
    }   // end for
    return true;
}   // end _extractFiles


bool QTools::moveFiles( const QString &src, const QString &dst, const QString &bck)
{
    bool ok = true;
    if ( QFileInfo(src).isDir())
    {
        QDir().mkpath(dst); // Does nothing if already exists
        QDir().mkpath(bck); // Does nothing if already exists
        for ( const QString &nm : QDir(src).entryList( QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
            if ( !(ok = moveFiles( src + "/" + nm, dst + "/" + nm, bck + "/" + nm)))
                break;
        if ( ok)
            ok = QDir().rmdir(src); // Remove the source directory
    }   // end if
    else
    {
        if ( QFileInfo::exists(dst))
            ok = QFile(dst).rename(bck);
        if ( ok)
            ok = QFile(src).rename(dst);
    }   // end else
    return ok;
}   // end moveFiles

