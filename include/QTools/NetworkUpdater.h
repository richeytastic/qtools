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

#include "UpdateMeta.h"
#include <QNetworkAccessManager>


namespace QTools {

class QTools_EXPORT NetworkUpdater : public QObject
{ Q_OBJECT
public:
    // Default timeout is 10 seconds with max of five redirects.
    NetworkUpdater( const QUrl& manifestUrl, int timeoutMsecs=10000, int maxRedirects=5);
    ~NetworkUpdater() override;

    // Return the manifest URL this object was constructed with.
    const QUrl &manifestUrl() const { return _manifestUrl;}

    // Returns the nature of any download error.
    inline const QString &error() const { return _err;}

    // Return the last update metadata downloaded from a manifest.
    inline const UpdateMeta& meta() const { return _meta;}

    // Refresh the manifest from the url given in the constructor and fire onReplyFinished when done.
    // Returns true iff the manifest was accessed and downloading was started.
    bool refreshManifest();

    // Download the update file and save to the given file location.
    // Emits onDownloadProgress until done then onReplyFinished.
    // Returns true iff the download was started (may return false
    // if still downloading something else).
    bool downloadUpdate( const QString&);

    // Returns true iff a connection is not yet finished.
    bool isBusy() const;

signals:
    // Signal that the network has finished replying.
    // Passes true iff response was received okay.
    void onReplyFinished( bool);

    // Signal the number of bytes received so far of the total number of bytes.
    // Total number of bytes is -1 if not known.
    void onDownloadProgress( qint64, qint64);

private slots:
    void _doOnReplyFinished();

private:
    const QUrl _manifestUrl;
    const int _toutMsecs;
    const int _maxRedirects;
    QNetworkAccessManager *_nman;
    QNetworkReply *_netr;
    UpdateMeta _meta;
    QString _err, _uname;

    bool _parseManifestReply( const std::string&);
    void _startConnection( const QUrl&, bool);
    NetworkUpdater( const NetworkUpdater&) = delete;
    void operator=( const NetworkUpdater&) = delete;
};  // end class

}   // end namespace

#endif
