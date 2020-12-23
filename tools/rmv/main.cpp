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

#include <QTemporaryDir>
#include <QFileInfo>
#include <iostream>

namespace {
bool moveFiles( const QString &src, const QString &dst, const QString &bck)
{
    bool ok = true;
    if ( QFileInfo(src).isDir())
    {
        QDir().mkpath(dst); // Does nothing if already exists
        QDir().mkpath(bck); // Does nothing if already exists
        for ( const QString &nm : QDir(src).entryList( QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
            if ( !(ok = moveFiles( src + "/" + nm, dst + "/" + nm, bck + "/" + nm)))
                break;
        if ( ok)    // Remove the source directory since now empty
            ok = QDir().rmdir(src);
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


int doMove( const QString &src, const QString &dst, const QString &bck)
{
    if (!QFileInfo::exists(src))
    {
        std::cerr << "Source does not exist!" << std::endl;
        return EXIT_FAILURE;
    }   // end if

    if (QFileInfo::exists(bck))
    {
        std::cerr << "Backup location must not already exist!" << std::endl;
        return EXIT_FAILURE;
    }   // end if

    int rv = EXIT_SUCCESS;
    if ( !moveFiles( src, dst, bck))
    {
        rv = EXIT_FAILURE;
        std::cerr << "Restoring failed move!" << std::endl;
        if (!moveFiles( bck, dst, src))
        {
            std::cerr << "Restore failed!" << std::endl;
            rv = EXIT_FAILURE;
        }   // end if
    }   // end if
    return rv;
}   // end doMove


void printHelp()
{
    std::cerr << "Use this tool ONLY with QTools::FileIO::moveFilesAsRoot." << std::endl;
    std::cerr << "Set QTools::FileIO::FILE_MOVE_TOOL with this tool's path." << std::endl;
}   // end printHelp


// Validity checking string to ensure we have the right tool
bool isValidChk( const QString &chk)
{
    return chk == ",.afdf63,f803c,,3b[]()";
}   // end isValidChk

}   // end namespace


int main( int argc, char *argv[])
{
    if ( argc >= 2 && std::string(argv[1]) == "--help")
    {
        printHelp();
        return EXIT_FAILURE;
    }   // end if

    if ( !isValidChk( argv[argc-1]))
        return EXIT_FAILURE;

    const QString src = argv[1];
    int exitCode = EXIT_SUCCESS;
    if (argc == 5)  // Move files?
    {
        const QString dst = argv[2];
        const QString bck = argv[3];
        exitCode = doMove( src, dst, bck);
    }   // end if
    else if ( !QFile::remove(src))
        exitCode = EXIT_FAILURE;

    return exitCode;
}   // end main

