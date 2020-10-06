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


int main( int argc, char *argv[])
{
    if ( argc < 3)
    {
        std::cerr << "Pass in source and destination and optionally, a backup location!" << std::endl;
        return EXIT_FAILURE;
    }   // end if

    const QString src = argv[1];
    if (!QFileInfo::exists(src))
    {
        std::cerr << "Source does not exist!" << std::endl;
        return EXIT_FAILURE;
    }   // end if

    int exitCode = EXIT_SUCCESS;
    const QString dst = argv[2];

    QTemporaryDir tmpBckDir;    // Default backup location
    QString bck = tmpBckDir.path();
    if ( argc > 3)  // User specified backup location?
    {
        bck = argv[3];
        if (QFileInfo::exists(bck))
        {
            std::cerr << "Specified backup location must not already exist!" << std::endl;
            return EXIT_FAILURE;
        }   // end if
    }   // end if

    if ( !moveFiles( src, dst, bck))
    {
        exitCode = EXIT_FAILURE;
        std::cerr << "Move failed! Restoring..." << std::endl;
        if (!moveFiles( bck, dst, src))
        {
            std::cerr << "Restore failed!" << std::endl;
            exitCode = EXIT_FAILURE;
        }   // end if
    }   // end if

    return exitCode;
}   // end main
