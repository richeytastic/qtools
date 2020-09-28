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

#ifndef QTOOLS_PATCH_LIST_H
#define QTOOLS_PATCH_LIST_H

#include "QTools_Export.h"
#include <boost/property_tree/ptree.hpp>
#include <QByteArray>
#include <QMap>
#include <QUrl>
#include <list>

namespace QTools {

class QTools_EXPORT PatchFiles
{
public:
    PatchFiles();
    PatchFiles( const PatchFiles&) = default;
    PatchFiles& operator=( const PatchFiles&) = default;

    bool setArchive( const QString&);
    const QString &archive() const { return _archive;}

    bool addFile( const QString&);
    const QStringList &files() const { return _files;}

private:
    QString _archive;
    QStringList _files;
};  // end class


class QTools_EXPORT PatchMeta
{
public:
    PatchMeta();
    PatchMeta( const PatchMeta&) = default;
    PatchMeta& operator=( const PatchMeta&) = default;

    // Returns true iff at least one version number
    // is greater than zero and a base URL is present.
    bool isValid() const;

    // Operations that compare patch versions.
    bool operator<( const PatchMeta&) const;
    bool operator==( const PatchMeta&) const;
    bool operator!=( const PatchMeta&) const;
    bool operator>( const PatchMeta&) const;
    bool operator<=( const PatchMeta&) const;
    bool operator>=( const PatchMeta&) const;

    // Get/set version numbers.
    int major() const { return _major;}
    int minor() const { return _minor;}
    int patch() const { return _patch;}
    bool setMajor( int);
    bool setMinor( int);
    bool setPatch( int);

    // Get/set details about this version.
    const QString &description() const { return _deets;}
    bool setDescription( const QString&);

    // Get/set the base URL of the patch.
    const QString &baseUrl() const { return _baseUrl;}
    bool setBaseUrl( const QString&);

    // Construct and return the full patch URL for this patch for this platform.
    QUrl patchUrl() const;

    void setManifest( const PatchFiles &v) { _platform = v;}

private:
    int _major, _minor, _patch;
    QString _deets;
    QString _baseUrl;
    PatchFiles _platform;
};  // end class


class QTools_EXPORT PatchList
{
public:
    PatchList();
    PatchList( const PatchList&) = default;
    PatchList& operator=( const PatchList&) = default;

    // Returns true iff there are patches in this list.
    bool hasPatches() const { return !_patches.empty();}

    // Returns true iff there's at least one patch available that will bring
    // the app (with version as given by the parameters) to a higher version.
    bool isPatchAvailable( int major, int minor, int patch) const;

    // Returns a description of the patch as a concatenation of descriptions
    // from all of the patches that are a higher version than the given version.
    QString patchDescription( int major, int minor, int patch) const;

    // Return a list of the patch URLs needed with the most recent first.
    // Returns all patches by default (all patches have versions > 0.0.0).
    QList<QUrl> patchURLs( int major=0, int minor=0, int patch=0) const;

    // Try to parse the given array returning true iff succeeded.
    // On return of false, call error() to return the error string.
    bool parse( const QByteArray&);

    // Return the parse error.
    const QString &error() const { return _err;}

    // Returns the name of the application.
    const QString &appName() const { return _appName;}

    // Return the application target directory to extract patches to
    // as a relative path from QCoreApplication::applicationDirPath().
    const QString &appTargetDir() const { return _appTgtDir;}

private:
    QString _appName;
    QString _appTgtDir;
    QString _err;
    QList<PatchMeta> _patches;
    bool _parsePatchMeta( const boost::property_tree::ptree&);
    bool _parsePatchFiles( PatchMeta&, const boost::property_tree::ptree&);
};  // end class

}   // end namespace

#endif