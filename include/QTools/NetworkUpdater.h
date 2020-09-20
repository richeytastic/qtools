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

#ifndef QTOOLS_NETWORK_UPDATER_H
#define QTOOLS_NETWORK_UPDATER_H

#include "UpdateMeta.h"
#include <QNetworkAccessManager>
#include <QTemporaryFile>

namespace QTools {

class QTools_EXPORT NetworkUpdater : public QObject
{ Q_OBJECT
public:
    /**
     * Provide the location of the manifest to be checking, and
     * where old files are moved to when performing an update.
     * Default network timeout is 10 seconds and a maximum of
     * 5 redirects are allowed.
     */
    NetworkUpdater( const QUrl& manifestUrl, const QString &oldDirPath,
                    int timeoutMsecs=10000, int maxRedirects=5);
    ~NetworkUpdater() override;

    // Return the manifest URL this object was constructed with.
    const QUrl &manifestUrl() const { return _manifestUrl;}

    // Returns the nature of any error.
    inline const QString &error() const { return _err;}

    // Return the last update metadata downloaded from a manifest.
    inline const UpdateMeta& meta() const { return _meta;}

    // Refresh manifest with constructor url and fire onRefreshedManifest when done.
    // Returns true iff the manifest URL was accessed and downloading was started.
    bool refreshManifest();

    // Begin downloading the update file. Emits onDownloadProgress until done
    // then onFinishedDownloadingUpdate. Returns true iff the download was started.
    bool downloadUpdate();

    // Start updating the app given the downloaded update file. Emits signal
    // onFinishedUpdate when complete. Returns true iff the update was started
    // and will return false if there's no update file yet.
    bool updateApp();

    // Returns true iff a network connection is active or an update is ongoing.
    bool isBusy() const;

    bool isUpdatingAllowed() const;

signals:
    void onRefreshedManifest();

    // Signal the number of bytes received so far of the total number of bytes.
    // Total number of bytes is -1 if not known.
    void onDownloadProgress( qint64, qint64);

    void onFinishedDownloadingUpdate();

    void onFinishedUpdate();

    // Emitted for any errors from asynchronous operations.
    void onError( const QString&);

private slots:
    void _doOnReplyFinished();
    void _doOnFinishedUpdate( const QString&);

private:
    const QUrl _manifestUrl;
    const QString _olddir;
    QNetworkRequest _nreq;
    QNetworkAccessManager *_nman;
    QNetworkReply *_netr;
    bool _isUpdating;
    UpdateMeta _meta;
    QString _err;
    QTemporaryFile _ufile;

    bool _parseManifestReply( const std::string&);
    void _startConnection( const QUrl&, bool);
    NetworkUpdater( const NetworkUpdater&) = delete;
    void operator=( const NetworkUpdater&) = delete;
};  // end class

}   // end namespace

#endif
