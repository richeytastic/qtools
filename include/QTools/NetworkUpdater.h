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

#ifndef QTOOLS_NETWORK_UPDATER_H
#define QTOOLS_NETWORK_UPDATER_H

#include "VersionInfo.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>


namespace QTools {

class QTools_EXPORT NetworkUpdater : public QObject
{ Q_OBJECT
public:
    explicit NetworkUpdater( QObject *parent = nullptr);
    ~NetworkUpdater() override;

    // Download the version manifest from the give url. Will signal onFinishedManifestDownload.
    void downloadManifest( const QUrl&);

    // Returns the nature of any manifest download error.
    inline const QString &error() const { return _err;}

    inline const VersionInfo& version() const { return _vers;}

signals:
    // Signal that the network request for downloading the version manifest has finished.
    // Passes true iff a valid manifest file was downloaded and parsed properly.
    void onFinishedManifestDownload( bool);

private slots:
    void _doOnNetworkReplied( QNetworkReply*);

private:
    QNetworkAccessManager *_nman;
    VersionInfo _vers;
    QString _err;

    void _reset();
    bool _parseReply( const std::string&);
    void _setError( QNetworkReply::NetworkError);
    NetworkUpdater( const NetworkUpdater&) = delete;
    void operator=( const NetworkUpdater&) = delete;
};  // end class

}   // end namespace

#endif
