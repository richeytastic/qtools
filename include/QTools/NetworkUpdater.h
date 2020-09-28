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

#include "PatchList.h"
#include <QNetworkAccessManager>
#include <QTemporaryFile>

namespace QTools {

class QTools_EXPORT NetworkUpdater : public QObject
{ Q_OBJECT
public:
    /**
     * Provide the URL of the patch list manifest to use, and where old files
     * are moved to when performing an update. Default network timeout is
     * 10 seconds and a maximum of 5 redirects are allowed.
     */
    NetworkUpdater( const QUrl& manifestUrl, const QString &oldDirPath,
                    int timeoutMsecs=10000, int maxRedirects=5);
    ~NetworkUpdater() override;

    // Returns true iff a network connection is active or an update is ongoing.
    bool isBusy() const;

    // Returns the nature of any error.
    inline const QString &error() const { return _err;}

    // Refresh manifest from constructor URL and emit onRefreshedManifest when done.
    // Returns true iff the manifest URL was accessed and downloading was started.
    bool refreshManifest();

    // Call after refreshing the patch manifest. Returns true iff an
    // update exists that will bring the app to a higher version.
    bool isUpdateAvailable( int major, int minor, int patch) const;

    // Returns a description of the updates available from the given version.
    QString updateDescription( int major, int minor, int patch) const;

    // Begin downloading the updates. Emits onDownloadProgress until done then
    // onFinishedDownloadingUpdates. Returns true iff downloading was started.
    // Only the updates that bring the app to a higher version are downloaded.
    bool downloadUpdates( int major, int minor, int patch);

    // Start updating the app given the downloaded data. Emits onFinishedUpdate
    // when complete. Returns true iff the update was started and returns false
    // if no data to update with have been downloaded yet.
    // Pass in the path to the appimagetool for repackaging if necessary.
    // If the running application is an AppImage and the passed in path is
    // invalid, this function returns false and sets an error to say that
    // updates cannot be made since any updates would not be able to be
    // repackaged back into the app's AppImage.
    bool updateApp( const QString &appImageToolPath="");

signals:
    void onRefreshedManifest();

    // Signal the number of bytes received so far of the total number of bytes.
    // Total number of bytes is -1 if not known.
    void onDownloadProgress( qint64, qint64);

    // Emitted once all updates have finished downloading.
    void onFinishedDownloadingUpdates();

    // Emitted in sequence after calling updateApp.
    // Signal onStartedRepackagingApp may not be emitted if not needed on this platform.
    void onStartedExtractingUpdates();
    void onStartedUpdatingFiles();
    void onStartedRepackagingApp();
    void onFinishedUpdating();

    // Emitted for any errors from asynchronous operations.
    void onError( const QString&);

private slots:
    void _doOnReplyFinished( QNetworkReply*);
    void _doOnDownloadProgress( QNetworkReply*);
    void _doOnFinishedUpdating( const QString&);

private:
    const QUrl _manifestUrl;
    const QString _olddir;
    QNetworkRequest _templateReq;
    QNetworkAccessManager *_nman;
    bool _isManifest;
    bool _isDownloading;
    bool _isUpdating;
    PatchList _plist;
    QList<QNetworkReply*> _nconns;
    QList<QTemporaryFile*> _files;
    QString _err;

    void _deleteFiles();
    bool _writeDataToFile( QNetworkReply*);
    bool _allRepliesFinished() const;
    bool _allUpdatesDownloaded() const;
    void _deleteConnections();
    QNetworkReply *_startConnection( const QUrl&, bool);
    NetworkUpdater( const NetworkUpdater&) = delete;
    void operator=( const NetworkUpdater&) = delete;
};  // end class

}   // end namespace

#endif
