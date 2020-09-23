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
#include <rlib/StringUtil.h>
#include <boost/property_tree/xml_parser.hpp>
#include <QNetworkReply>
#include <QDataStream>
#include <QFileInfo>
using QTools::NetworkUpdater;
using PTree = boost::property_tree::ptree;


NetworkUpdater::NetworkUpdater( const QUrl &url, const QString &olddir, int tmsecs, int mr)
    : _manifestUrl(url), _olddir(olddir), _nman( nullptr), _netr(nullptr), _isUpdating(false)
{
    _nman = new QNetworkAccessManager(this);
    _nreq.setAttribute( QNetworkRequest::CacheSaveControlAttribute, false);   // Don't cache
    _nreq.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork); // Refresh
    _nreq.setAttribute( QNetworkRequest::FollowRedirectsAttribute, mr > 0);
    _nreq.setMaximumRedirectsAllowed( mr);
    _nreq.setTransferTimeout( tmsecs);
    AppUpdater::recordAppExe();
}   // end ctor


NetworkUpdater::~NetworkUpdater()
{
    if ( _nman)
        delete _nman;
}   // end dtor


bool NetworkUpdater::isUpdatingAllowed() const
{
    bool isok = true;
#ifdef __linux__
    isok = AppUpdater::isAppImage();
#endif
    return isok;
}   // end isUpdatingAllowed


bool NetworkUpdater::isBusy() const
{
    return _netr != nullptr || _isUpdating;
}   // end isBusy


void NetworkUpdater::_startConnection( const QUrl &url, bool emitProgress)
{
    _nreq.setUrl(url);
    _netr = _nman->get( _nreq);
    connect( _netr, &QNetworkReply::errorOccurred, [this](){ _err = _netr->errorString();});
    connect( _netr, &QNetworkReply::finished, this, &NetworkUpdater::_doOnReplyFinished);
    if ( emitProgress)  // Simply forward through the signal
        connect( _netr, &QNetworkReply::downloadProgress, this, &NetworkUpdater::onDownloadProgress);
}   // end _startConnection


void NetworkUpdater::_doOnReplyFinished()
{
    const bool isManifest = !_meta.isValid();
    bool ok = _err.isEmpty() && _netr->bytesAvailable();
    if ( ok)
    {
        if ( isManifest)
            ok = _parseManifestReply( _netr->readAll().toStdString());
        else
        {
            QFile ufile( _ufile.fileName());
            if ( ufile.open( QIODevice::WriteOnly))
            {
                QDataStream out( &ufile);
                const QByteArray bytes = _netr->readAll();
                if ( out.writeRawData( bytes.constData(), bytes.size()) < 0)
                {
                    _err = tr("Unable to write downloaded update to file!");
                    ok = false;
                }   // end if
            }   // end if
            else
            {
                _err = tr("Unable to open temporary file for writing!");
                ok = false;
            }   // end else
        }   // end else
    }   // end if

    _netr->deleteLater();
    _netr = nullptr;

    if ( !ok)
    {
        _removeUpdateFile();
        if ( _err.isEmpty())
            _err = tr("Unable to connect to resource!");
        emit onError(_err);
    }   // end if
    else if ( isManifest)
        emit onRefreshedManifest();
    else
        emit onFinishedDownloadingUpdate();
}   // end _doOnReplyFinished


void NetworkUpdater::_removeUpdateFile()
{
    if ( !_ufile.fileName().isEmpty())
        _ufile.remove();    // Remove any existing update file
}   // end _removeUpdateFile


bool NetworkUpdater::refreshManifest()
{
    if ( isBusy())
    {
        _err = tr("Updater is busy!");
        return false;
    }   // end if
    _meta = UpdateMeta();  // Reset
    _removeUpdateFile();
    _startConnection( _manifestUrl, false/*don't emit progress updates*/);
    return true;
}   // end refreshManifest


bool NetworkUpdater::downloadUpdate()
{
    if ( isBusy())
    {
        _err = tr("Updater is busy!");
        return false;
    }   // end if
    if ( _meta.isValid())
    {
        _err = tr("No valid update available!");
        return false;
    }   // end if
    _removeUpdateFile();
    if ( !_ufile.open())
    {
        _err = tr("Unable to open temporary file!");
        return false;
    }   // end if
    _startConnection( _meta.updateUrl(), true/*emit progress updates*/);
    return true;
}   // end downloadUpdate


bool NetworkUpdater::updateApp()
{
    if ( isBusy())
    {
        _err = tr("Updater is busy!");
        return false;
    }   // end if
    if ( _ufile.fileName().isEmpty())
    {
        _err = tr("Downloading of update file not yet started!");
        return false;
    }   // end if
    if ( QFileInfo( _ufile.fileName()).size() == 0)
    {
        _err = tr("Update file not finished downloading!");
        return false;
    }   // end if

    AppUpdater *updater = new AppUpdater( _ufile.fileName(), _olddir, _meta.updateTarget());
    connect( updater, &AppUpdater::onFinished, this, &NetworkUpdater::_doOnFinishedUpdate);
    connect( updater, &AppUpdater::finished, updater, &QObject::deleteLater);
    updater->start();
    return true;
}   // end updateApp


void NetworkUpdater::_doOnFinishedUpdate( const QString &err)
{
    if ( !err.isEmpty())
        emit onError( err);
    else
        emit onFinishedUpdate();
}   // end _doOnFinishedUpdate


namespace {

QString missingContentTags( const PTree &vdata)
{
    QStringList mtags;

    if ( vdata.count("Name") == 0)
        mtags << "Name";

    if ( vdata.count("Major") == 0)
        mtags << "Major";

    if ( vdata.count("Minor") == 0)
        mtags << "Minor";

    if ( vdata.count("Patch") == 0)
        mtags << "Patch";

    if ( vdata.count("Details") == 0)
        mtags << "Details";

#ifdef _WIN32
    if ( vdata.count("Windows") == 0)
        mtags << "Windows";
#elif __linux__
    if ( vdata.count("Linux") == 0)
        mtags << "Linux";
#else
    mtags << "OS";
#endif

    QString err;
    if ( !mtags.isEmpty())
        err = "Missing " + mtags.join("; ") + " tag(s)!";

    return err;
}   // end missingContentTags

}   // end namespace


bool NetworkUpdater::_parseManifestReply( const std::string &xmldata)
{
    std::istringstream iss( xmldata);
    PTree tree;

    try
    {
        boost::property_tree::read_xml( iss, tree);
    }   // end try
    catch ( const boost::property_tree::ptree_bad_path&)
    {
        _err = tr("XML bad path!");
    }   // end 
    catch ( const boost::property_tree::xml_parser_error&)
    {
        _err = tr("XML parse error!");
    }   // end catch
    catch ( const std::exception&)
    {
        _err = tr("Unspecified XML parse error!");
    }   // end catch

    if ( !_err.isEmpty())
        return false;

    if ( tree.count("VersionManifest") == 0)
    {
        _err = "VersionManifest tag not found!";
        return false;
    }   // end if

    const PTree &vdata = tree.get_child("VersionManifest");
    _err =  missingContentTags( vdata);
    if ( !_err.isEmpty())
        return false;

    const PTree *ftree = nullptr;
#ifdef _WIN32
    ftree = &vdata.get_child("Windows");
#elif __linux__
    ftree = &vdata.get_child("Linux");
#endif
    assert( ftree);

    if ( ftree->count("Update") == 0)
        _err = "Missing Update tag!";
    else
    {
        _meta.setUpdateUrl( QUrl( QString::fromStdString( rlib::trim( ftree->get<std::string>("Update")))));
        // The target directory is given relative to the application directory path
        const PTree &utree = ftree->get_child("Update");
        boost::optional<std::string> tgt = utree.get_optional<std::string>("<xmlattr>.targetdir");
        _meta.setUpdateTarget( tgt ? QString::fromStdString( *tgt) : "");
    }   // end else

    if ( _err.isEmpty())
    {
        _meta.setName( QString::fromStdString( rlib::trim( vdata.get<std::string>("Name"))));
        _meta.setMajor( vdata.get<int>("Major"));
        _meta.setMinor( vdata.get<int>("Minor"));
        _meta.setPatch( vdata.get<int>("Patch"));
        _meta.setDetails( QString::fromStdString( rlib::trim( vdata.get<std::string>("Details"))));
    }   // end if
     
    return _err.isEmpty();
}   // end _parseManifestReply
