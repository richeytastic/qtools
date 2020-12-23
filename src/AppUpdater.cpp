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
#include <FileIO.h> // QTools
#include <quazip5/JlCompress.h>
#include <QCoreApplication>
#include <iostream>
using QTools::AppUpdater;

namespace {

using namespace QTools;
/*
void _printFileInfo( const QString &pth)
{
    const QFileInfo finfo( pth);
    const QString owner = finfo.owner();
    const QString group = finfo.group();
    const QString rights = FileIO::permissionsString( pth);
    std::cerr << "FilePath:    " << pth.toLocal8Bit().toStdString() << std::endl;
    std::cerr << "   Owner:    " << owner.toStdString() << std::endl;
    std::cerr << "   Group:    " << group.toStdString() << std::endl;
    std::cerr << "   Rights:   " << rights.toStdString() << std::endl;
    std::cerr << "   In home:  " << std::boolalpha << FileIO::inHomeDir( pth) << std::endl;
    std::cerr << "   Is owner: " << std::boolalpha << (owner == FileIO::username()) << std::endl;
}   // end _printFileInfo
*/

bool _isFileAllowed( const QString &f, const QString &username)
{
    return FileIO::inHomeDir(f) || (QFileInfo(f).owner() == username);
}   // end _isAllowed


bool _isAllowed( const QStringList &flist)
{
    const QString username = FileIO::username();
    for ( const QString &f : flist)
        if ( !_isFileAllowed( f, username))
            return false;
    return true;
}   // end _isAllowed


bool _updateFiles( const QString &src, const QString &tgt, const QString &bck)
{
    std::cerr << "[INFO] QTools::AppUpdater: Updating \"" << tgt.toLocal8Bit().toStdString() << "\"" << std::endl;
    // Write directly directory if we have permission. Otherwise
    // invoke via process to allow OS to request permissions.
    bool ok = true;
    if ( FileIO::isRoot() || _isAllowed( {src, tgt}))
        ok = FileIO::moveFiles( src, tgt, bck);
    else
        ok = FileIO::moveFilesAsRoot( src, tgt, bck);

    if ( !ok)
        std::cerr << "[WARNING] QTools::AppUpdater: Unable to update - file locks?" << std::endl;
    return ok;
}   // end _updateFiles


void _removeFiles( const QStringList &rpaths, const QString &tgt)
{
    const QString username = FileIO::username();
    const bool isRoot = FileIO::isRoot();
    for ( const QString &pth : rpaths)
    {
        bool ok = false;
        const QString fpath = QFileInfo( tgt + "/" + pth).canonicalFilePath();
        if ( isRoot || _isFileAllowed( fpath, username))
            ok = QFile::remove( fpath);
        else
            ok = FileIO::removeFileAsRoot( fpath);

        if (!ok)
        {
            std::cerr << "[WARNING] QTools::AppUpdater: Unable to remove \""
                      << fpath.toLocal8Bit().toStdString() << "\"" << std::endl;
        }   // end if
    }   // end for
}   // end _removeFiles


}   // end namespace


AppUpdater::AppUpdater()
{
    _appFilePath = QCoreApplication::applicationFilePath();
    // On Linux, recording the information below gives the location of the AppImage
    // if the application is in that format while QCoreApplication::applicationFilePath()
    // returns the exe in the temporary filesystem mounted by the AppImage.
    // This gives a method to check if the application is AppImage.
#ifdef __linux__
    QFile cmdline("/proc/self/cmdline");
    if ( cmdline.open(QIODevice::ReadOnly | QIODevice::Text))
        _appFilePath = QFileInfo( QString( cmdline.readAll()).trimmed()).canonicalFilePath();
#endif
    //std::cerr << "AppFilePath: " << _appFilePath.toLocal8Bit().toStdString() << std::endl;
}   // end ctor


bool AppUpdater::_isAppImage() const
{
#ifdef _WIN32
    return false;
#endif
    return QCoreApplication::applicationFilePath() != _appFilePath;
}   // end _isAppImage


void AppUpdater::setAppPatchDir( const QString &rp) { _relPath = rp;}


bool AppUpdater::update( const QStringList &fns, const QStringList &rpaths)
{
    if ( _isAppImage() && FileIO::APP_IMAGE_TOOL.isEmpty())
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

    _rpaths = rpaths;

    start();
    return true;
}   // end update


void AppUpdater::run()
{
    // Create the scratch directory and required entries
    static const QString APP_NAME = QCoreApplication::applicationName();
    // The scratch directory has the application and the username since
    // if multiple users have their own version of the application this
    // allows different backup directories to exist alongside each other.
    static const QString SCRATCH_DIR = QDir::tempPath() + QString("/%1_%2_safe_to_delete").arg(APP_NAME).arg(FileIO::username());
    static const QString BACKUPS_DIR = SCRATCH_DIR + "/Backups";
    static const QString EXTRACT_DIR = SCRATCH_DIR + "/Extract";
    static const QString NEW_APP_DIR = SCRATCH_DIR + "/AppDir";
    QDir( SCRATCH_DIR).removeRecursively();  // Remove this directory if present from previous runs

    emit onExtracting();
    if ( !_extractFiles( EXTRACT_DIR))
        return _failFinish( "Failed to extract archive!");

    // If this is an AppImage, files are mounted read-only so copy
    // everything to a new location and update there before repacking.
    static QString binDir = QCoreApplication::applicationDirPath();
    if ( _isAppImage())
    {
        static const QString APP_DIR = QDir( binDir + "/../..").canonicalPath();
        std::cerr << "[INFO] QTools::AppUpdater: Copying "
            << APP_DIR.toStdString() << " to " << NEW_APP_DIR.toStdString() << std::endl;
        if ( !FileIO::copyFiles( APP_DIR, NEW_APP_DIR))
            return _failFinish( "Failed to copy app dir to temp dir!");
        binDir = QDir( NEW_APP_DIR + "/usr/bin").canonicalPath();
    }   // end if

    emit onUpdating();
    const QString PATCH_DIR = QDir( binDir + "/" + _relPath).canonicalPath();
    if ( !_updateFiles( EXTRACT_DIR, PATCH_DIR, BACKUPS_DIR))
        return _failFinish( "Failed to update files!");

    // Don't fail if files aren't removed.
    _removeFiles( _rpaths, PATCH_DIR);

    // Repackage the updated application directory as an AppImage?
    if ( _isAppImage())
    {
        emit onRepacking();
        static const QString NEW_APP_IMG = SCRATCH_DIR + QString("/%1-NEW.AppImage").arg(APP_NAME);
        static const QString OLD_APP_IMG = SCRATCH_DIR + QString("/%1-OLD.AppImage").arg(APP_NAME);
        _err = _repackAppImage( NEW_APP_DIR, NEW_APP_IMG, OLD_APP_IMG);
    }   // end if

    std::cerr << "[INFO] QTools::AppUpdater: Finished" << std::endl;
    emit onFinished( _err);
}   // end run


void AppUpdater::_failFinish( const char *err)
{
    _err = tr(err);
    emit onFinished(err);
}   // end _failFinish


bool AppUpdater::_extractFiles( const QString &xdir) const
{
    // Extract all files from each archive into the same temporary directory
    // in reverse order. This ensures that the newer files with the same names
    // clobber the older files with the same names.
    for ( int i = _fpaths.size() - 1; i >= 0; --i)
    {
        std::cerr << "[INFO] QTools::AppUpdater: Extracting \"" << _fpaths.at(i).toLocal8Bit().toStdString() << "\"" << std::endl;
        const QStringList flst = JlCompress::extractDir( _fpaths.at(i), xdir);
        if ( flst.size() == 0)
            return false;
    }   // end for
    return true;
}   // end _extractFiles


QString AppUpdater::_repackAppImage( const QString &NEW_APP_DIR, const QString &NEW_APP_IMG, const QString &OLD_APP_IMG) const
{
    std::cerr << "[INFO] QTools::AppUpdater: Repacking AppImage..." << std::endl;
    if ( !FileIO::packAppImage( NEW_APP_DIR, NEW_APP_IMG))
        return tr("Failed to repack AppImage!");

    // Swap the new AppImage for the existing one. Since the existing one
    // is locked, move it to scratch before replacing with the new one.
    QString err;
    if ( FileIO::isRoot() || _isAllowed( {NEW_APP_IMG, _appFilePath}))
        err = FileIO::swapOverFiles( NEW_APP_IMG, _appFilePath, OLD_APP_IMG);
    else
        err = FileIO::swapOverFilesAsRoot( NEW_APP_IMG, _appFilePath, OLD_APP_IMG);  // LINUX ONLY!
    return err;
}   // end _repackAppImage
