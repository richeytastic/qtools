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
using PTree = boost::property_tree::ptree;


NetworkUpdater::NetworkUpdater( const QUrl &url, const QString &olddir, int tmsecs, int mr)
    : _manifestUrl(url), _nman( nullptr), _isManifest(false)
{
    _nman = new QNetworkAccessManager(this);
    //_templateReq.setAttribute( QNetworkRequest::CacheSaveControlAttribute, false);   // Don't cache
    //_templateReq.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork); // Refresh
    _templateReq.setAttribute( QNetworkRequest::FollowRedirectsAttribute, mr > 0);
    _templateReq.setMaximumRedirectsAllowed( mr);
    _templateReq.setTransferTimeout( tmsecs);

    _updater.recordAppExe();
    _updater.setDeleteDir( olddir);
    connect( &_updater, &AppUpdater::onStartedExtraction, this, &NetworkUpdater::onStartedExtractingUpdates);
    connect( &_updater, &AppUpdater::onStartedMovingFiles, this, &NetworkUpdater::onStartedUpdatingFiles);
    connect( &_updater, &AppUpdater::onStartedRepackaging, this, &NetworkUpdater::onStartedRepackagingApp);
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

    _plist.setCurrentVersion( mj, mn, pt);  // Can't be set lower
    _isManifest = true;
    _deleteFiles();
    _nconns.push_back( _startConnection( _manifestUrl, false/*don't emit progress updates*/));
    return true;
}   // end refreshManifest


bool NetworkUpdater::isUpdateAvailable() const { return _plist.hasPatches();}

QString NetworkUpdater::updateDescription() const { return _plist.patchDescription();}


bool NetworkUpdater::downloadUpdates()
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

    _deleteFiles();
    const QList<QUrl> urls = _plist.patchURLs();
    for ( const QUrl &url : urls)
    {
        _nconns.push_back( _startConnection( url, true/*emit progress updates*/));
        _files.push_back( nullptr);  // Corresponding position of the file
    }   // end for

    return true;
}   // end downloadUpdates


void NetworkUpdater::_deleteFiles()
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
}   // end _deleteFiles


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
    emit onDownloadProgress( bytesRecv, totalBytes);
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


void NetworkUpdater::_deleteConnections()
{
    _isManifest = false;
    for ( QNetworkReply *nr : _nconns)
        nr->deleteLater();
    _nconns.clear();
}   // end _deleteConnections


void NetworkUpdater::_doOnReplyFinished( QNetworkReply *nconn)
{
    bool ok = _err.isEmpty() && nconn->bytesAvailable() > 0;
    if ( ok)
    {
        if ( _isManifest)
        {
            ok = _plist.parse( nconn->readAll());
            _err = _plist.error();  // Will be empty if ok
            _deleteConnections();
            if (ok)
            {
                _updater.setAppTargetDir( _plist.appTargetDir());
                emit onRefreshedManifest();
            }   // end else
        }   // end if
        else if ( _allRepliesFinished())
        {
            for ( QNetworkReply *nc : _nconns)
                ok |= _writeDataToFile( nc);
            _deleteConnections();
            if ( ok)
                emit onFinishedDownloadingUpdates();
        }   // end else if
    }   // end if

    if ( !ok)
    {
        if ( _err.isEmpty())
            _err = tr("Unable to connect to resource!");
        _deleteConnections();
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


bool NetworkUpdater::updateApp( const QString &appImageToolPath)
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

    // Note that this will prevent updating if running through a debugger...
    if ( _updater.isAppImage() && !_updater.setAppImageToolPath( appImageToolPath))
    {
        _err = tr("Invalid appimagetool path given!");
        return false;
    }   // end if

    QStringList fnames;
    for ( const QTemporaryFile *file : _files)
        fnames.append( file->fileName());
    _updater.setFiles( fnames);
    _updater.start();
    return true;
}   // end updateApp


void NetworkUpdater::_doOnFinishedUpdating( const QString &err)
{
    _deleteFiles();
    if ( !err.isEmpty())
        emit onError( err);
    else
    {
        _plist.setCurrentVersion( _plist.highestVersion());
        emit onFinishedUpdating();
    }   // end else
}   // end _doOnFinishedUpdating
