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

    start();
    return true;
}   // end update


void AppUpdater::run()
{
    // Create the scratch directory and required entries
    static const QString APP_NAME = QCoreApplication::applicationName();
    static const QString SCRATCH_DIR = QDir::tempPath() + QString("/%1_safe_to_delete").arg(APP_NAME);
    static const QString BACKUPS_DIR = SCRATCH_DIR + "/Backups";
    static const QString EXTRACT_DIR = SCRATCH_DIR + "/Extract";
    static const QString NEW_APP_DIR = SCRATCH_DIR + "/AppDir";
    static const QString NEW_APP_IMG = SCRATCH_DIR + QString("/%1-NEW.AppImage").arg(APP_NAME);
    static const QString OLD_APP_IMG = SCRATCH_DIR + QString("/%1-OLD.AppImage").arg(APP_NAME);
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

    // Repackage the updated application directory as an AppImage?
    if ( _isAppImage())
    {
        emit onRepacking();
        if ( !_repackAppImage( NEW_APP_DIR, NEW_APP_IMG, OLD_APP_IMG))
            return _failFinish( "Unable to repack new AppImage!");
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
    std::cerr << "[INFO] QTools::AppUpdater: Extracting archives..." << std::endl;
    // Extract all files from each archive into the same temporary directory
    // in reverse order. This ensures that the newer files with the same names
    // clobber the older files with the same names.
    for ( int i = _fpaths.size() - 1; i >= 0; --i)
    {
        const QStringList flst = JlCompress::extractDir( _fpaths.at(i), xdir);
        if ( flst.size() == 0)
            return false;
    }   // end for
    return true;
}   // end _extractFiles


bool AppUpdater::_updateFiles( const QString &xdir, const QString &pdir, const QString &bdir) const
{
    std::cerr << "[INFO] QTools::AppUpdater: Updating files in \""
        << pdir.toLocal8Bit().toStdString() << "\"" << std::endl;
    bool ok = true;
    // Write into the application patch directory if we have permission.
    // Otherise invoke via detached process to allow OS to request permission.
    if ( FileIO::canWrite( pdir))
        ok = FileIO::moveFiles( xdir, pdir, bdir);
    else if ( !FileIO::isRoot())
        ok = FileIO::moveFilesAsRoot( xdir, pdir, bdir);
    else
    {
        std::cerr << "[WARNING] QTools::AppUpdater: Unable to update - file locks?" << std::endl;
        ok = false;
    }   // end else
    return ok;
}   // end _updateFiles


bool AppUpdater::_repackAppImage( const QString &NEW_APP_DIR,
                                  const QString &NEW_APP_IMG,
                                  const QString &OLD_APP_IMG) const
{
    static const std::string WRNSTR = "[WARNING] QTools::AppUpdater: Failed to ";

    std::cerr << "[INFO] QTools::AppUpdater: Repacking AppImage..." << std::endl;
    if ( !FileIO::packAppImage( NEW_APP_DIR, NEW_APP_IMG))
    {
        std::cerr << WRNSTR << "repack AppImage!" << std::endl;
        return false;
    }   // end if

    // Swap the new AppImage for the existing one. Since the existing one
    // is locked, move it to scratch before replacing with the new one.
    if ( !QFile::rename( _appFilePath, OLD_APP_IMG))
    {
        std::cerr << WRNSTR << "move existing AppImage to scratch!" << std::endl;
        return false;
    }   // end if

    if ( !QFile::rename( NEW_APP_IMG, _appFilePath))
    {
        std::cerr << WRNSTR << "move fresh AppImage to original location!" << std::endl;
        return false;
    }   // end if

    return true;
}   // end _repackAppImage
