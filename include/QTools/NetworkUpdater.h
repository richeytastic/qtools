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

#include "AppUpdater.h"
#include "PatchList.h"
#include <QNetworkAccessManager>
#include <QTemporaryFile>

namespace QTools {

class QTools_EXPORT NetworkUpdater : public QObject
{ Q_OBJECT
public:
    /**
     * Provide the URL of the patch list manifest to use.
     * Default network timeout is 10 seconds and a maximum of 5 redirects are allowed.
     */
    NetworkUpdater( const QUrl& manifestUrl, int timeoutMsecs=10000, int maxRedirects=5);
    ~NetworkUpdater() override;

    // Returns true iff a network connection is active or an update is ongoing.
    bool isBusy() const;

    // Returns the nature of any error.
    inline const QString &error() const { return _err;}

    // Refresh manifest from constructor URL and emit onRefreshedManifest when done.
    // Pass in the current version which will be checked against the manifest when
    // downloaded to see if there are any updates available. Returns true iff the
    // manifest URL was accessed and downloading was started. Returns false if
    // currently busy or if the user lacks sufficient privileges to update.
    bool refreshManifest( int major, int minor, int patch);

    // Call after refreshing the patch manifest. Returns true iff an
    // update exists that will bring the app to a higher version.
    bool isUpdateAvailable() const;

    // Returns a description of any available updates or an empty
    // string if no updates are available.
    QString updateDescription() const;

    // Start downloading updates and updating the app. Emits onFinishedUpdating
    // when complete. Returns true if updating was started.
    bool updateApp();

signals:
    void onRefreshedManifest();

    // Signal the percentage data downloaded so far. Will be -1 if not known.
    void onDownloadProgress( double);

    // Emitted when downloading of updates has finished but updating itself
    // is yet to finish. Signal onFinishedUpdating is emitted after this either
    // immediately or some time later (depending on the method of update).
    void onFinishedDownloading();

    // Emitted as soon as updating has finished.
    void onFinishedUpdating();

    // Emitted for any errors from asynchronous operations.
    void onError( const QString&);

private slots:
    void _doOnReplyFinished( QNetworkReply*);
    void _doOnDownloadProgress( QNetworkReply*);
    void _doOnFinishedUpdating( const QString&);

private:
    const QUrl _manifestUrl;
    QNetworkRequest _templateReq;
    QNetworkAccessManager *_nman;
    bool _isManifest;
    PatchList _plist;
    QList<QNetworkReply*> _nconns;
    QList<QTemporaryFile*> _files;
    QString _err;
    AppUpdater _updater;

    bool _writeDataToFile( QNetworkReply*);
    bool _allRepliesFinished() const;
    bool _allUpdatesDownloaded() const;
    void _resetConnections();
    void _resetDownloads();
    bool _startAppUpdater();
    QNetworkReply *_startConnection( const QUrl&, bool);
    NetworkUpdater( const NetworkUpdater&) = delete;
    void operator=( const NetworkUpdater&) = delete;
};  // end class

}   // end namespace

#endif
