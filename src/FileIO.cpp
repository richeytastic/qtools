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

#include <FileIO.h>
#include <QProcess>
#include <QFileInfo>
#include <QTemporaryDir>
#include <iostream>

#ifdef __linux__    // For getuid and geteuid
#include <unistd.h>
#include <sys/types.h>
#endif

// Definitions for these namespace variables
QString QTools::FileIO::APP_IMAGE_TOOL;
QString QTools::FileIO::FILE_MOVE_TOOL;


namespace {
bool _moveFiles( const QString &src, const QString &dst, const QString &bck)
{
    bool ok = true;
    if ( QFileInfo(src).isDir())
    {
        QDir().mkpath(dst); // Does nothing if already exists
        QDir().mkpath(bck); // Does nothing if already exists
        for ( const QString &nm : QDir(src).entryList( QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
            if ( !(ok = _moveFiles( src + "/" + nm, dst + "/" + nm, bck + "/" + nm)))
                break;
        if ( ok)
            ok = QDir().rmdir(src); // Remove the source directory
    }   // end if
    else
    {
        if ( QFileInfo::exists(dst))
            ok = QFile::rename(dst, bck);
        if ( ok)
            ok = QFile::rename(src, dst);
    }   // end else
    return ok;
}   // end _moveFiles


bool _copyFiles( const QString &src, const QString &dst, QList<QFileInfo> &symLinks, bool noclobber)
{
    static const std::string WRNSTR = "[WARNING] QTools::FileInfo: Unable to ";

    bool ok = true;
    const QFileInfo sinfo(src);

    if ( sinfo.isDir())
    {
        QDir().mkpath(dst); // Does nothing if already exists
        for ( const QString &nm : QDir(src).entryList( QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
            if ( !(ok = _copyFiles( src + "/" + nm, dst + "/" + nm, symLinks, noclobber)))
                break;
    }   // end if
    else if ( sinfo.isSymbolicLink() || sinfo.isShortcut())
        symLinks.append(sinfo);
    else
    {
        if ( QFileInfo::exists(dst) && !noclobber && !QFile::remove(dst))
        {
            std::cerr << WRNSTR << "remove \"" << dst.toLocal8Bit().toStdString() << "\"!" << std::endl;
            ok = false;
        }   // end if

        if ( ok && !QFile::copy( src, dst))
        {
            std::cerr << WRNSTR << "copy \"" << dst.toLocal8Bit().toStdString() << "\" - file exists!" << std::endl;
            ok = false;
        }   // end if
    }   // end else

    return ok;
}   // end _copyFiles

}   // end namespace


bool QTools::FileIO::copyFiles( const QString &src, const QString &dst, bool noclobber)
{
    QList<QFileInfo> symLinks;
    if ( !_copyFiles( src, dst, symLinks, noclobber))
        return false;

    if ( symLinks.isEmpty())    // No symlinks/shortcuts so we're done!
        return true;

    // Deal with recreating all symlinks/shortcuts now that the files are copied across
    const QString sAbsPth = QFileInfo(src).absoluteFilePath();
    const QString dAbsPth = QFileInfo(dst).absoluteFilePath();
    for ( const QFileInfo &sinfo : symLinks)
    {
        QString lnkSrc = sinfo.absoluteFilePath();
        lnkSrc.replace( sAbsPth, dAbsPth);  // lnkSrc will end with ".lnk" on Windows

        // The link target may not be inside src - if so this replacement has no effect.
        QString lnkTgt = QFileInfo( sinfo.symLinkTarget()).absoluteFilePath();
        lnkTgt.replace( sAbsPth, dAbsPth);

        if ( !QFile::link( lnkTgt, lnkSrc))
            return false;
    }   // end for

    return true;
}   // end copyFiles


bool QTools::FileIO::moveFiles( const QString &src, const QString &dst, const QString &ubck)
{
    QString bck = ubck;
    QTemporaryDir tdir;
    if ( bck.isEmpty())
    {
        if (!tdir.isValid())
        {
            std::cerr << "Unable to create valid temporary directory!" << std::endl;
            return false;
        }   // end if
        bck = tdir.path();
    }   // end if

    bool ok = true;
    if ( !_moveFiles( src, dst, bck))
    {
        std::cerr << "[WARNING] QTools::FileIO::moveFiles: Move failed - restoring..." << std::endl;
        if ( !_moveFiles( bck, dst, src))
            std::cerr << "[WARNING] QTools::FileIO::moveFiles: Restore failed!! Possible data loss!" << std::endl;
        ok = false;
    }   // end if

    return ok;
}   // end moveFiles


namespace {
#ifdef _WIN32
// To check file permissions on Windows
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;

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


QString toolPath( const QString &atool)
{
    const QFileInfo file(atool);
    return file.exists( atool) && file.isExecutable() ? file.canonicalFilePath() : "";
}   // end toolPath

}   // end namespace


bool QTools::FileIO::isRoot()
{
    bool isr = false;
#ifdef _WIN32
    isr = isRunningAsAdmin();
#elif __linux__
    isr = getuid() != geteuid();
#endif
    return isr;
}   // end isRoot


bool QTools::FileIO::canWrite( const QString &path)
{
    return QFileInfo::exists( path) && (QFile::permissions(path) & QFileDevice::WriteUser);
}   // end canWrite


bool QTools::FileIO::moveFilesAsRoot( const QString &src, const QString &dst, const QString &bck)
{
    QString program = toolPath(FILE_MOVE_TOOL);
    if ( program.isEmpty())
        return false;

    // Validity checking string to ensure we have the right tool
    static const QString chk = ",.afdf63,f803c,,3b[]()";

    QStringList args;
#ifdef __linux__
    args << program << src << dst << bck << chk;
    program = "pkexec";
#elif _WIN32
    args << "-Command" << "Start-Process"
         << QString("'%1'").arg(program)
         << QString("'\"%1\" \"%2\" \"%3\" \"%4\"'").arg(src).arg(dst).arg(bck).arg(chk)
         << "-Verb" << "runAs";
    program = "powershell";
#endif

    return QProcess::execute( program, args) == 0;
}   // end moveFilesAsRoot


bool QTools::FileIO::packAppImage( const QString &appDir, const QString &repackfile)
{
    const QString appImgTool = toolPath(APP_IMAGE_TOOL);
    if ( appImgTool.isEmpty())
        return false;
    QStringList args;
    args << "-n" << appDir << repackfile;
    return QProcess::execute( appImgTool, args) == 0;
}   // end packAppImage
