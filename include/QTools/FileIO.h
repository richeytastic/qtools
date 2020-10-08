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

#ifndef QTOOLS_FILE_IO_H
#define QTOOLS_FILE_IO_H

#include "QTools_Export.h"
#include <QString>

namespace QTools {
namespace FileIO {

// Recursively move files and directories from src to dst placing any
// existing destination files in the given backup location (bck).
// Returns true on successful move of all src files to dst and the files
// at the backup location may be discarded. False is returned if any
// of the source files could not be moved and the file system is restored
// to the state it was in before calling this function.
QTools_EXPORT bool moveFiles( const QString &src, const QString &dst, const QString &bck="");

// Moves files using an external tool (set as the path FILE_MOVE_TOOL)
// which is started in a child process via an OS mechanism to prompt
// the user to provide administrator (root) privileges.
QTools_EXPORT bool moveFilesAsRoot( const QString &src, const QString &dst, const QString &bck="");

// Recursively copy files from src to dst. By default, fails if any
// files at dst already exist otherwise set noclobber false to overwrite.
QTools_EXPORT bool copyFiles( const QString &src, const QString &dst, bool noclobber=true);

// Move file f1 to f2, then move file f0 to f1.
// File f2 must not already exist and files f0 and f1 must exist.
// Returns an empty string on success otherwise it contains the error.
QTools_EXPORT QString swapOverFiles( const QString &f0, const QString &f1, const QString &f2);

// As above but execute as root (LINUX ONLY CURRENTLY!)
QTools_EXPORT QString swapOverFilesAsRoot( const QString &f0, const QString &f1, const QString &f2);

// Run the AppImage packaging process on the given appDir to produce the
// given destination appImageFile. Runs as a separate process with current
// user privileges. Requires the APP_IMAGE_TOOL path to be set.
QTools_EXPORT bool packAppImage( const QString &appDir, const QString &appImageFile);

// Checks on Windows if the current user has administator privileges
// or on Linux if the current user is root (or rather that their
// effective user ID is different from the actual user ID).
QTools_EXPORT bool isRoot();

// Reliably returns the username.
QTools_EXPORT QString username();

// Is the given path within the user's home directory?
QTools_EXPORT bool inHomeDir( const QString &path);

// Returns a UNIX style permissions string with rwx flags for owner, group, and other.
QTools_EXPORT QString permissionsString( const QString &path);

QTools_EXPORT extern QString APP_IMAGE_TOOL; // Path to the appimagetool-x86_64.AppImage.
QTools_EXPORT extern QString FILE_MOVE_TOOL; // Path to the file move tool (bin/rmv).

}}   // end namespaces
     
#endif
