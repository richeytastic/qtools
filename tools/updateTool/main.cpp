/************************************************************************
 * Copyright (C) 2021 Richard Palmer
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

}   // end namespace


int main( int argc, char *argv[])
{
    if ( argc < 2 || ",.afdf63,f803c,,3b[]()" != std::string(argv[1]))
    {
        std::cerr << "Use programmatically only." << std::endl;
        std::cerr << "Set QTools::FileIO::UPDATE_TOOL with this tool's path." << std::endl;
        return EXIT_FAILURE;
    }   // end if

    const std::string cmd = argv[2];

    int exitCode = EXIT_SUCCESS;
    if ( cmd == "move")
    {
        if ( argc != 6)
            return EXIT_FAILURE;
        exitCode = doMove( argv[3], argv[4], argv[5]);
    }   // end if
    else if ( cmd == "remove")
    {
        if ( argc < 4)
            return EXIT_FAILURE;
        for ( int i = 3; i < argc; ++i)
            QFile::remove( argv[i]);
    }   // end else if
    else
    {
        std::cerr << "Invalid update command! Use \"move\" or \"remove\" only." << std::endl;
        exitCode = EXIT_FAILURE;
    }   // end else

    return exitCode;
}   // end main

