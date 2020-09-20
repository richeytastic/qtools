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

#ifndef QTOOLS_UPDATE_META_H
#define QTOOLS_UPDATE_META_H

#include "QTools_Export.h"
#include <QUrl>

namespace QTools {

class QTools_EXPORT UpdateMeta
{
public:
    UpdateMeta();
    UpdateMeta( const UpdateMeta&) = default;
    UpdateMeta& operator=( const UpdateMeta&) = default;

    // Returns true iff at least one version number is greater than zero
    // and an update URL is present.
    bool isValid() const;

    // Operations that compare version numbers.
    bool operator<( const UpdateMeta&) const;
    bool operator==( const UpdateMeta&) const;
    bool operator!=( const UpdateMeta&) const;
    bool operator>( const UpdateMeta&) const;
    bool operator<=( const UpdateMeta&) const;
    bool operator>=( const UpdateMeta&) const;

    // Get/set the name of the application.
    const QString &name() const { return _name;}
    void setName( const QString &n) { _name = n;}

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

    // Get/set URLs to the installer, update, and source.
    const QUrl &installUrl() const { return _installUrl;}
    const QUrl &updateUrl() const { return _updateUrl;}
    const QUrl &sourceUrl() const { return _sourceUrl;}
    void setInstallUrl( const QUrl &v) { _installUrl = v;}
    void setUpdateUrl( const QUrl &v) { _updateUrl = v;}
    void setSourceUrl( const QUrl &v) { _installUrl = v;}

    // The update target given relative to the application directory path.
    void setUpdateTarget( const QString &v) { _updateTarget = v;}
    const QString &updateTarget() const { return _updateTarget;}

private:
    int _major, _minor, _patch;
    QString _name, _deets;
    QUrl _installUrl, _updateUrl, _sourceUrl;
    QString _updateTarget;
};  // end class

}   // end namespace

#endif
