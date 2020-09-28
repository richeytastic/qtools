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
using QTools::NetworkUpdater;
using PTree = boost::property_tree::ptree;


NetworkUpdater::NetworkUpdater( const QUrl &url, const QString &olddir, int tmsecs, int mr)
    : _manifestUrl(url), _olddir(olddir),
      _nman( nullptr), _isManifest(false), _isDownloading(false), _isUpdating(false)
{
    _nman = new QNetworkAccessManager(this);
    _templateReq.setAttribute( QNetworkRequest::CacheSaveControlAttribute, false);   // Don't cache
    _templateReq.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork); // Refresh
    _templateReq.setAttribute( QNetworkRequest::FollowRedirectsAttribute, mr > 0);
    _templateReq.setMaximumRedirectsAllowed( mr);
    _templateReq.setTransferTimeout( tmsecs);
    AppUpdater::recordAppExe();
}   // end ctor


NetworkUpdater::~NetworkUpdater() { delete _nman;}


bool NetworkUpdater::isBusy() const { return _isManifest || _isDownloading || _isUpdating;}


bool NetworkUpdater::refreshManifest()
{
    if ( isBusy())
    {
        _err = tr("Updater is busy!");
        return false;
    }   // end if
    _plist = PatchList();  // Reset
    _isManifest = true;
    _deleteFiles();
    _nconns.push_back( _startConnection( _manifestUrl, false/*don't emit progress updates*/));
    return true;
}   // end refreshManifest


bool NetworkUpdater::isUpdateAvailable( int mj, int mn, int pt) const { return _plist.isPatchAvailable( mj, mn, pt);}

QString NetworkUpdater::updateDescription( int mj, int mn, int pt) const { return _plist.patchDescription( mj, mn, pt);}


bool NetworkUpdater::downloadUpdates( int mj, int mn, int pt)
{
    if ( isBusy())
    {
        _err = tr("Updater is busy!");
        return false;
    }   // end if

    if ( !isUpdateAvailable( mj, mn, pt))
    {
        _err = tr("No update is available!");
        return false;
    }   // end if

    _deleteFiles();
    _isDownloading = true;
    const QList<QUrl> urls = _plist.patchURLs( mj, mn, pt);
    for ( const QUrl &url : urls)
    {
        _nconns.push_back( _startConnection( url, true/*emit progress updates*/));
        _files.push_back( nullptr);  // Corresponding position of the file
    }   // end for

    return true;
}   // end downloadUpdates


void NetworkUpdater::_deleteFiles()
{
    _isUpdating = false;
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
    _isDownloading = false;
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
            if ( !ok)
                _err = _plist.error();
        }   // end if
        else
            ok = _writeDataToFile( nconn);
    }   // end if

    if ( !ok)
    {
        if ( _err.isEmpty())
            _err = tr("Unable to connect to resource!");
        _deleteConnections();
        emit onError(_err);
    }   // end if
    else if ( _isManifest)
    {
        _deleteConnections();
        emit onRefreshedManifest();
    }   // end else if
    else if ( _allRepliesFinished())
    {
        _deleteConnections();
        emit onFinishedDownloadingUpdates();
    }   // end else if
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

    if ( AppUpdater::isAppImage() && !AppUpdater::setAppImageToolPath( appImageToolPath))
    {
        _err = tr("Invalid appimagetool path given!");
        return false;
    }   // end if

    _isUpdating = true;
    QStringList fnames;
    for ( const QTemporaryFile *file : _files)
        fnames.append( file->fileName());
    AppUpdater *updater = new AppUpdater( fnames, _plist.appTargetDir(), _olddir);
    connect( updater, &AppUpdater::onStartedExtraction, this, &NetworkUpdater::onStartedExtractingUpdates);
    connect( updater, &AppUpdater::onStartedMovingFiles, this, &NetworkUpdater::onStartedUpdatingFiles);
    connect( updater, &AppUpdater::onStartedRepackaging, this, &NetworkUpdater::onStartedRepackagingApp);
    connect( updater, &AppUpdater::onFinished, this, &NetworkUpdater::_doOnFinishedUpdating);
    connect( updater, &AppUpdater::finished, updater, &QObject::deleteLater);
    updater->start();
    return true;
}   // end updateApp


void NetworkUpdater::_doOnFinishedUpdating( const QString &err)
{
    _deleteFiles();
    if ( !err.isEmpty())
        emit onError( err);
    else
        emit onFinishedUpdating();
}   // end _doOnFinishedUpdating
