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

#include <QTools/NetworkUpdater.h>
#include <rlib/StringUtil.h>
#include <boost/property_tree/xml_parser.hpp>
#include <QNetworkReply>
#include <QDataStream>
#include <QFile>
using QTools::NetworkUpdater;
using PTree = boost::property_tree::ptree;


NetworkUpdater::NetworkUpdater( const QUrl &url, int tmsecs, int mr)
    : _manifestUrl(url), _toutMsecs(tmsecs), _maxRedirects(mr), _nman( nullptr), _netr(nullptr)
{
    _nman = new QNetworkAccessManager(this);
}   // end ctor


NetworkUpdater::~NetworkUpdater()
{
    if ( _nman)
        delete _nman;
}   // end dtor


bool NetworkUpdater::isBusy() const { return _netr != nullptr;}


void NetworkUpdater::_startConnection( const QUrl &url, bool emitProgress)
{
    QNetworkRequest req(url);
    req.setAttribute( QNetworkRequest::CacheSaveControlAttribute, false);   // Don't cache
    req.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork); // Refresh
    req.setAttribute( QNetworkRequest::FollowRedirectsAttribute, _maxRedirects > 0);
    req.setMaximumRedirectsAllowed( _maxRedirects);
    req.setTransferTimeout( _toutMsecs);
    _netr = _nman->get( req);
    connect( _netr, &QNetworkReply::errorOccurred, [this](){ _err = _netr->errorString();});
    connect( _netr, &QNetworkReply::finished, this, &NetworkUpdater::_doOnReplyFinished);
    if ( emitProgress)
        connect( _netr, &QNetworkReply::downloadProgress, this, &NetworkUpdater::onDownloadProgress);
}   // end _startConnection


void NetworkUpdater::_doOnReplyFinished()
{
    bool ok = _err.isEmpty() && _netr->bytesAvailable();
    if ( ok)
    {
        if ( _uname.isEmpty())  // Manifest?
        {
            _meta = UpdateMeta();  // Reset
            ok = _parseManifestReply( _netr->readAll().toStdString());
        }   // end else if
        else
        {
            QFile ufile( _uname);
            if ( ufile.open( QIODevice::WriteOnly))
            {
                QDataStream out( &ufile);
                const QByteArray bytes = _netr->readAll();
                out.writeRawData( bytes.constData(), bytes.size());
            }   // end if
            else
            {
                _err = "Unable to write downloaded update to file!";
                ok = false;
            }   // end else
        }   // end else
    }   // end if

    _uname = QString();
    _netr->deleteLater();
    _netr = nullptr;
    emit onReplyFinished( ok);
}   // end _doOnReplyFinished


bool NetworkUpdater::refreshManifest()
{
    if ( isBusy())
        return false;
    _uname = QString();
    _startConnection( _manifestUrl, false/*don't emit progress updates*/);
    return true;
}   // end refreshManifest


bool NetworkUpdater::downloadUpdate( const QString &uname)
{
    if ( isBusy())
        return false;
    _uname = uname;
    if ( _uname.isEmpty())
    {
        _err = "Invalid update save filepath!";
        return false;
    }   // end if
    _startConnection( _meta.updateUrl(), true/*emit progress updates*/);
    return true;
}   // end downloadUpdate


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

    if ( vdata.count("Source") == 0)
        mtags << "Source";

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
        _err = "XML bad path!";
    }   // end 
    catch ( const boost::property_tree::xml_parser_error&)
    {
        _err = "XML parse error!";
    }   // end catch
    catch ( const std::exception&)
    {
        _err = "Unspecified XML parse error!";
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

    if ( ftree->count("Install") == 0)
        _err = "Missing Install tag!";
    else if ( ftree->count("Update") == 0)
        _err = "Missing Update tag!";
    else
    {
        _meta.setInstallUrl( QUrl( QString::fromStdString( rlib::trim( ftree->get<std::string>("Install")))));
        _meta.setUpdateUrl( QUrl( QString::fromStdString( rlib::trim( ftree->get<std::string>("Update")))));
    }   // end else

    if ( _err.isEmpty())
    {
        _meta.setName( QString::fromStdString( rlib::trim( vdata.get<std::string>("Name"))));
        _meta.setMajor( vdata.get<int>("Major"));
        _meta.setMinor( vdata.get<int>("Minor"));
        _meta.setPatch( vdata.get<int>("Patch"));
        _meta.setDetails( QString::fromStdString( rlib::trim( vdata.get<std::string>("Details"))));
        _meta.setSourceUrl( QUrl( QString::fromStdString( rlib::trim( vdata.get<std::string>("Source")))));
    }   // end if
     
    return _err.isEmpty();
}   // end _parseManifestReply
