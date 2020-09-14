/************************************************************************
 * Copyright (C) 2020 Richard Palmer
 *
 * Cliniface is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cliniface is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#ifndef QTOOLS_VERSION_INFO_H
#define QTOOLS_VERSION_INFO_H

#include "QTools_Export.h"
#include <QUrl>

namespace QTools {

class QTools_EXPORT VersionInfo
{
public:
    VersionInfo();
    VersionInfo( const VersionInfo&) = default;
    VersionInfo& operator=( const VersionInfo&) = default;

    // Operations that compare version numbers
    bool operator<( const VersionInfo&) const;
    bool operator==( const VersionInfo&) const;
    bool operator!=( const VersionInfo&) const;
    bool operator>( const VersionInfo&) const;
    bool operator<=( const VersionInfo&) const;
    bool operator>=( const VersionInfo&) const;

    // Get/set version numbers.
    int major() const { return _major;}
    int minor() const { return _minor;}
    int patch() const { return _patch;}
    void setMajor( int v) { _major = v;}
    void setMinor( int v) { _minor = v;}
    void setPatch( int v) { _patch = v;}

    // Get/set details about this version.
    const QString &details() const { return _deets;}
    void setDetails( const QString &v) { _deets = v;}

    // Get/set URLs to the installer and archive versions.
    const QUrl &installUrl() const { return _installUrl;}
    const QUrl &archiveUrl() const { return _archiveUrl;}
    void setInstallUrl( const QUrl &v) { _installUrl = v;}
    void setArchiveUrl( const QUrl &v) { _archiveUrl = v;}

private:
    int _major, _minor, _patch;
    QString _deets;
    QUrl _installUrl;
    QUrl _archiveUrl;
};  // end class

}   // end namespace

#endif
