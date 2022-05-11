/************************************************************************
 * Copyright (C) 2022 Richard Palmer
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
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTextStream>
#include <iostream>

#ifdef __linux__    // For getuid and geteuid
#include <unistd.h>
#include <sys/types.h>
#endif

// Definitions for these namespace variables
QString QTools::FileIO::APP_IMAGE_TOOL;
QString QTools::FileIO::UPDATE_TOOL;


namespace {
// Validity checking string to ensure we have the right tool
static const QString CHK_STR = ",.afdf63,f803c,,3b[]()";


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


bool _copyFiles( const QString &src, const QString &dst, QFileInfoList &symLinks, bool noclobber)
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


void _recursivelyListFiles( const QDir &dir, const QStringList &nameFilters, QFileInfoList &files)
{
    const QFileInfoList fentries = dir.entryInfoList( nameFilters, QDir::Files | QDir::Readable);
    for ( const QFileInfo &finfo : fentries)
        files.append(finfo);
    const QFileInfoList dentries = dir.entryInfoList( QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Readable);
    for ( const QFileInfo &finfo : dentries)
    {
        const QDir dpath( finfo.filePath());
        _recursivelyListFiles( dpath, nameFilters, files);
    }   // end for
}   // end _recursivelyListFiles

}   // end namespace


QFileInfoList QTools::FileIO::recursivelyListFiles( const QDir &root, const QStringList &nameFilters)
{
    QFileInfoList files;
    _recursivelyListFiles( root, nameFilters, files);
    return files;
}   // end recursivelyListFiles


QTools::FileIO::BackgroundFilesFinder::BackgroundFilesFinder( const QDir &root, const QStringList &nameFilters)
    : _root(root), _nameFilters(nameFilters) {}


void QTools::FileIO::BackgroundFilesFinder::run()
{
    QFileInfoList files;
    _recursivelyListFiles( _root, _nameFilters, files);
    emit onFoundFiles( _root, files);
}   // end run


bool QTools::FileIO::copyFiles( const QString &src, const QString &dst, bool noclobber)
{
    QFileInfoList symLinks;
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
        QString lnkTgt = sinfo.symLinkTarget();

        lnkSrc.replace( sAbsPth, dAbsPth);  // lnkSrc will end with ".lnk" on Windows

        // The link target may not be inside src - if so this replacement has no effect.
        // lnkTgt is assumed to be inside src, but the target path might include src
        // so replace. This will have no effect if src is not in the target path.
        lnkTgt.replace( sAbsPth, dAbsPth);

        // Finally, ensure the target is relative to the destination.
        lnkTgt = QDir( dAbsPth).relativeFilePath( lnkTgt);

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

void _writeTestFile( const QStringList &slst)
{
    QFile tfile( QDir::temp().filePath( "qtools_fileio_checkSwapFiles.txt"));
    tfile.remove();
    tfile.open( QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&tfile);
    for ( const QString &s : slst)
        out << s << Qt::endl;
    out.flush();
    tfile.close();
    std::cerr << "Error details at: " << QFileInfo(tfile).absoluteFilePath().toStdString() << std::endl;
}   // end _writeTestFile


QString _checkSwapFiles( const QString &fnew, const QString &fcur, const QString &fold)
{
    QString errMsg = "";
    if ( QFileInfo::exists( fold))
        errMsg = "Rename path already exists!";
    else if ( !QFileInfo::exists( fcur))
        errMsg = "Current file path does not exist!";
    else if ( !QFileInfo::exists( fnew))
        errMsg = "New file path does not exist!";
    if ( !errMsg.isEmpty())
    {
        _writeTestFile( {QString( "FNEW: %1").arg(fnew),
                        QString( "FCUR: %1").arg(fcur),
                        QString( "FOLD: %1").arg(fold)});
    }   // end if
    return errMsg;
}   // end _checkSwapFiles

}   // end namespace


QString QTools::FileIO::swapOverFiles( const QString &fnew, const QString &fcur, const QString &fold)
{
    const QString err = _checkSwapFiles( fnew, fcur, fold);
    if ( !err.isEmpty())
        return err;
    if ( !QFile::rename( fcur, fold))
        return "Failed to move current file to old!";
    if ( !QFile::rename( fnew, fcur))
        return "Failed to move new to current!";
    return "";
}   // end swapOverFiles


QString QTools::FileIO::swapOverFilesAsRoot( const QString &fnew, const QString &fcur, const QString &fold)
{
#ifdef _WIN32
    return "FUNCTION DEFINED ONLY FOR LINUX!";
#endif

    const QString err = _checkSwapFiles( fnew, fcur, fold);
    if ( !err.isEmpty())
        return err;

    // Create a temporary bash script to perform the shuffle
    QTemporaryDir tdir;
    if ( !tdir.isValid())
        return "Unable to create temporary directory!";

    QFile tfile( tdir.filePath("swapfiles.sh"));
    if ( !tfile.open(QIODevice::WriteOnly | QIODevice::Text))
        return "Unable to open script file!";

    const QString afnew = QFileInfo( fnew).absoluteFilePath();
    const QString afcur = QFileInfo( fcur).absoluteFilePath();
    const QString afold = QFileInfo( fold).absoluteFilePath();

    QTextStream out(&tfile);
    out << "#!/usr/bin/env sh" << Qt::endl;
    out << "mv -f " << afcur << " " << afold << Qt::endl;
    out << "mv -f " << afnew << " " << afcur << Qt::endl;
    out << "exit 0" << Qt::endl;
    out.flush();
    tfile.close();

    if ( !tfile.setPermissions( tfile.permissions() | QFileDevice::ExeOwner | QFileDevice::ExeOther | QFileDevice::ExeGroup))
        return "Unable to set executable permissions on temporary file for script!";

    const QString scriptFile = QFileInfo( tfile.fileName()).absoluteFilePath();
    const bool ok = QProcess::execute( "pkexec", {scriptFile}) == 0;
    return ok ? "" : "Process execution failed!";
}   // end swapOverFilesAsRoot


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


QString QTools::FileIO::username()
{
    QString user;
    // Use the following methods which are more reliable than
    // accessing environment variables user (UNIX) or username (Windows)
#ifdef _WIN32
    char wuname[256];
    DWORD nuname = sizeof(wuname);
    if (GetUserName(wuname, &nuname))
        user = wuname;
#elif __linux__
    QProcess process;
    process.setProcessChannelMode( QProcess::MergedChannels);
    process.start("whoami");
    if ( process.waitForFinished(-1))
        user = process.readAllStandardOutput();
#endif
    return user.trimmed();
}   // end username


bool QTools::FileIO::inHomeDir( const QString &path)
{
    return QFileInfo(path).canonicalFilePath().startsWith( QDir::homePath());
}   // end inHomeDir


namespace {
void appendPermChar( QString &fper, QChar c,
        const QFileDevice::Permissions p, const QFileDevice::Permissions ps)
{
    if ( ps & p)
        fper.append(c);
    else
        fper.append('-');
}   // end appendPermChar
}   // end namespace


QString QTools::FileIO::permissionsString( const QString &path)
{
    const QFileDevice::Permissions perms = QFile::permissions(path);
    QString fper;
    appendPermChar( fper, 'r', QFileDevice::ReadOwner, perms);
    appendPermChar( fper, 'w', QFileDevice::WriteOwner, perms);
    appendPermChar( fper, 'x', QFileDevice::ExeOwner, perms);
    appendPermChar( fper, 'r', QFileDevice::ReadGroup, perms);
    appendPermChar( fper, 'w', QFileDevice::WriteGroup, perms);
    appendPermChar( fper, 'x', QFileDevice::ExeGroup, perms);
    appendPermChar( fper, 'r', QFileDevice::ReadOther, perms);
    appendPermChar( fper, 'w', QFileDevice::WriteOther, perms);
    appendPermChar( fper, 'x', QFileDevice::ExeOther, perms);
    return fper;
}   // end permissionsString


bool QTools::FileIO::packAppImage( const QString &appDir, const QString &repackfile)
{
    const QString appImgTool = toolPath(APP_IMAGE_TOOL);
    if ( appImgTool.isEmpty())
        return false;

    qputenv( "ARCH", "x86_64");
    QStringList args;
    args << "-n" << appDir << repackfile;
    QProcess proc;
    proc.setStandardOutputFile( QProcess::nullDevice());
    //proc.setProcessChannelMode( QProcess::ForwardedChannels);
    proc.start( appImgTool, args);
    const bool finOkay = proc.waitForFinished(-1);
    //const int oval = QProcess::execute( appImgTool, args);
    qunsetenv( "ARCH");
    return finOkay && QFileInfo::exists( repackfile);
}   // end packAppImage


bool QTools::FileIO::moveFilesAsRoot( const QString &src, const QString &dst, const QString &bck)
{
    QString program = toolPath(UPDATE_TOOL);
    if ( program.isEmpty())
        return false;

    QStringList args;
#ifdef __linux__
    args << program << CHK_STR << "move" << src << dst << bck;
    program = "pkexec";
#elif _WIN32
    args << "-Command" << "Start-Process"
         << QString("'%1'").arg(program)
         << QString("'\"%1\" \"%2\" \"%3\" \"%4\" \"%5\"'").arg(CHK_STR).arg("move").arg(src).arg(dst).arg(bck)
         << "-Verb" << "runAs";
    program = "powershell";
#endif

    return QProcess::execute( program, args) == 0;
}   // end moveFilesAsRoot


bool QTools::FileIO::removeFileAsRoot( const QString &fl) { return removeFilesAsRoot( {fl});}


bool QTools::FileIO::removeFilesAsRoot( const QStringList &fls)
{
    QString program = toolPath(UPDATE_TOOL);
    if ( program.isEmpty())
        return false;

    QStringList args;
#ifdef __linux__
    args << program << CHK_STR << "remove";
    for ( const QString &fl : fls)
        args << fl;
    program = "pkexec";
#elif _WIN32
    args << "-Command" << "Start-Process"
         << QString("'%1'").arg(program)
         << QString("'\"%1\" \"remove\"").arg(CHK_STR);

    const int N = fls.size();
    for ( int i = 0; i < N-1; ++i)
        args << QString(" \"%1\"").arg(fls.at(i));
    args << QString(" \"%1\"'").arg(fls.last());

    args << "-Verb" << "runAs";
    program = "powershell";
#endif

    return QProcess::execute( program, args) == 0;
}   // end removeFilesAsRoot
