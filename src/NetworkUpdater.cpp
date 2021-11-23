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

#include <QTools/NetworkUpdater.h>
#include <QTools/AppUpdater.h>
#include <QNetworkReply>
#include <QDataStream>
#include <QFileInfo>
#include <iostream>
using QTools::NetworkUpdater;


NetworkUpdater::NetworkUpdater( const QUrl &url, int tmsecs, int mr)
    : _manifestUrl(url), _nman( nullptr), _isManifest(false)
{
    _nman = new QNetworkAccessManager(this);
    //_templateReq.setAttribute( QNetworkRequest::CacheSaveControlAttribute, false);   // Don't cache
    //_templateReq.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork); // Refresh
    _templateReq.setAttribute( QNetworkRequest::FollowRedirectsAttribute, mr > 0);
    _templateReq.setMaximumRedirectsAllowed( mr);
    _templateReq.setTransferTimeout( tmsecs);

    connect( &_updater, &AppUpdater::onFinished, this, &NetworkUpdater::_doOnFinishedUpdating);
}   // end ctor


NetworkUpdater::~NetworkUpdater() { delete _nman;}


bool NetworkUpdater::isBusy() const { return !_nconns.isEmpty() || _updater.isRunning();}


bool NetworkUpdater::refreshManifest( int mj, int mn, int pt)
{
    if ( isBusy())
    {
        _err = tr("Updater is busy!");
        return false;
    }   // end if

    /*
    if ( !isPrivileged())
    {
        _err = tr("User has insufficient privileges!");
        return false;
    }   // end if
    */

    _plist.setCurrentVersion( mj, mn, pt);  // Can't be set lower
    _resetDownloads();
    _isManifest = true;
    _nconns.push_back( _startConnection( _manifestUrl, false/*don't emit progress updates*/));
    _files.push_back( nullptr);  // Corresponding position of the file
    return true;
}   // end refreshManifest


bool NetworkUpdater::isUpdateAvailable() const { return _plist.hasPatches();}


QString NetworkUpdater::updateDescription() const
{
    return _plist.patchDescription();
}   // end updateDescription


void NetworkUpdater::_resetDownloads()
{
    for ( QTemporaryFile *tfile : _files)
    {
        if ( tfile)
        {
            tfile->remove();
            delete tfile;
        }   // end if
    }   // end for
    _files.clear();
    _resetConnections();
}   // end _resetDownloads


void NetworkUpdater::_resetConnections()
{
    _isManifest = false;
    for ( QNetworkReply *nr : _nconns)
        nr->deleteLater();
    _nconns.clear();
}   // end _resetConnections


QNetworkReply *NetworkUpdater::_startConnection( const QUrl &url, bool emitProgress)
{
    QNetworkRequest nreq = _templateReq;
    nreq.setUrl(url);
    QNetworkReply *nr = _nman->get( nreq);
    connect( nr, &QNetworkReply::errorOccurred, [=](){ _err = nr->errorString();});
    connect( nr, &QNetworkReply::finished, [=](){ _doOnReplyFinished( nr);});
    if ( emitProgress)
        connect( nr, &QNetworkReply::downloadProgress, [=](){ _doOnDownloadProgress( nr);});
    return nr;
}   // end _startConnection


void NetworkUpdater::_doOnDownloadProgress( QNetworkReply *nconn)
{
    qint64 totalBytes = 0;
    qint64 bytesRecv = 0;
    for ( const QNetworkReply *nr : _nconns)
    {
        bytesRecv += nr->bytesAvailable();
        const qlonglong tbs = nr->header( QNetworkRequest::ContentLengthHeader).toLongLong();

        if ( tbs <= 0)
            totalBytes = -1;
        if ( totalBytes >= 0)
            totalBytes += tbs;
    }   // end for
    double pcnt = -1;
    if ( totalBytes > 0)
        pcnt = 100.0 * double(bytesRecv) / totalBytes;
    emit onDownloadProgress( pcnt);
}   // end _doOnDownloadProgress


bool NetworkUpdater::_writeDataToFile( QNetworkReply *nconn)
{
    QTemporaryFile *tfile = new QTemporaryFile;
    if ( !tfile->open())
    {
        delete tfile;
        _err = tr("Unable to open temporary file to write downloaded data!");
    }   // end if
    else
    {
        _files[_nconns.indexOf(nconn)] = tfile; // Set the file at the ordered position
        QFile file( tfile->fileName());
        if ( file.open( QIODevice::WriteOnly))
        {
            QDataStream out( &file);
            const QByteArray bytes = nconn->readAll();
            if ( out.writeRawData( bytes.constData(), bytes.size()) < 0)
                _err = tr("Unable to write downloaded data to file!");
        }   // end if
    }   // end else

    return _err.isEmpty();
}   // end _writeDataToFile


bool NetworkUpdater::_allRepliesFinished() const
{
    for ( const QNetworkReply *nr : _nconns)
        if ( !nr->isFinished())
            return false;
    return true;
}   // end _allRepliesFinished


void NetworkUpdater::_doOnReplyFinished( QNetworkReply *nconn)
{
    bool ok = _err.isEmpty() && nconn->bytesAvailable() > 0;
    if ( ok)
    {
        if ( _isManifest)
        {
            ok = _writeDataToFile( nconn);
            if ( ok)
            {
                ok = _plist.parse( _files.first()->fileName());
                _err = _plist.error();  // Will be empty if ok
            }   // end if
            _resetDownloads();
            if (ok)
            {
                _updater.setAppPatchDir( _plist.appTargetDir());
                emit onRefreshedManifest();
            }   // end else
        }   // end if
        else if ( _allRepliesFinished())
        {
            for ( QNetworkReply *nc : _nconns)
                ok |= _writeDataToFile( nc);
            _resetConnections();
            if ( ok)
            {
                emit onFinishedDownloading();
                _startAppUpdater();
            }   // end if
        }   // end else if
    }   // end if

    if ( !ok)
    {
        if ( _err.isEmpty())
            _err = tr("Unable to connect to resource!");
        _resetDownloads();
        emit onError(_err);
    }   // end if
}   // end _doOnReplyFinished


bool NetworkUpdater::_allUpdatesDownloaded() const
{
    for ( QTemporaryFile *tfile : _files)
        if ( !tfile)
            return false;
    return !_files.isEmpty();
}   // end _allUpdatesDownloaded


bool NetworkUpdater::updateApp()
{
    if ( isBusy())
    {
        _err = tr("Updater is busy!");
        return false;
    }   // end if

    if ( !isUpdateAvailable())
    {
        _err = tr("No update is available!");
        return false;
    }   // end if

    _resetDownloads();

    // Otherwise we have to download all the updates first and start the updater later.
    const QList<QUrl> urls = _plist.patchURLs();
    for ( const QUrl &url : urls)
    {
        _nconns.push_back( _startConnection( url, true/*emit progress updates*/));
        _files.push_back( nullptr);  // Corresponding position of the file
    }   // end for
    return true;
}   // end updateApp


bool NetworkUpdater::_startAppUpdater()
{
    if ( isBusy())
    {
        _err = tr("Updater is busy!");
        return false;
    }   // end if

    if ( !_allUpdatesDownloaded())
    {
        _err = tr("Updates not yet downloaded!");
        return false;
    }   // end if

    // Collect the downloaded temporary patch archives into a string list
    QStringList fnames;
    for ( const QTemporaryFile *file : _files)
        fnames.append( file->fileName());

    // Files to remove (specified only from the latest patch!)
    const QStringList &rfiles = _plist.highestVersion().files().rfiles();

    // Run the update in a separate thread.
    _updater.update( fnames, rfiles);

    return true;
}   // end _startAppUpdater


void NetworkUpdater::_doOnFinishedUpdating( const QString &err)
{
    _resetDownloads();
    if ( !err.isEmpty())
        emit onError( err);
    else
    {
        _plist.setCurrentVersion( _plist.highestVersion());
        emit onFinishedUpdating();
    }   // end else
}   // end _doOnFinishedUpdating
