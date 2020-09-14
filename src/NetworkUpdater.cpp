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
#include <boost/property_tree/xml_parser.hpp>
using QTools::NetworkUpdater;
using PTree = boost::property_tree::ptree;


NetworkUpdater::NetworkUpdater( QObject *parent)
    : _nman( new QNetworkAccessManager(this))
{
    connect( _nman, &QNetworkAccessManager::finished, this, &NetworkUpdater::_doOnNetworkReplied);
}   // end ctor


NetworkUpdater::~NetworkUpdater() { delete _nman;}


void NetworkUpdater::downloadManifest( const QUrl &url)
{
    _err = QString();
    _vers = VersionInfo();
    QNetworkRequest req;
    req.setUrl( url);
    _nman->get( req);
}   // end downloadManifest


void NetworkUpdater::_doOnNetworkReplied( QNetworkReply *nr)
{
    bool ok = nr->error() == QNetworkReply::NoError;
    if ( !ok)
        _setError( nr->error());
    else
        ok = _parseReply( nr->readAll().toStdString());
    nr->deleteLater();
    emit onFinishedManifestDownload( ok);
}   // end _doOnNetworkReplied


namespace {

QString missingContentTags( const PTree &vdata)
{
    QStringList mtags;
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


bool NetworkUpdater::_parseReply( const std::string &xmldata)
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
    const QString missingTags =  missingContentTags( vdata);
    if ( !missingTags.isEmpty())
    {   
        _err = missingTags;
        return false;
    }   // end if

    const PTree *ftree = nullptr;
#ifdef _WIN32
    ftree = &vdata.get_child("Windows");
#elif __linux__
    ftree = &vdata.get_child("Linux");
#endif
    assert( ftree);

    if ( ftree->count("Install") == 0)
        _err = "Missing Install tag!";
    else if ( ftree->count("Archive") == 0)
        _err = "Missing Archive tag!";
    else
    {
        _vers.setInstallUrl( QUrl( QString::fromStdString( ftree->get<std::string>("Install"))));
        _vers.setArchiveUrl( QUrl( QString::fromStdString( ftree->get<std::string>("Archive"))));
    }   // end else

    if ( _err.isEmpty())
    {
        _vers.setMajor( vdata.get<int>("Major"));
        _vers.setMinor( vdata.get<int>("Minor"));
        _vers.setPatch( vdata.get<int>("Patch"));
        _vers.setDetails( QString::fromStdString( vdata.get<std::string>("Details")));
    }   // end if
     
    return _err.isEmpty();
}   // end _parseReply


void NetworkUpdater::_setError( QNetworkReply::NetworkError err)
{
    switch ( err)
    {
        case QNetworkReply::ConnectionRefusedError:
            _err = "Connection refused!";
            break;
        case QNetworkReply::HostNotFoundError:
            _err = "Host not found!";
            break;
        case QNetworkReply::TimeoutError:
            _err = "Connection timed out!";
            break;
        case QNetworkReply::ContentNotFoundError:
            _err = "Content not found (404)!";
            break;
        case QNetworkReply::ContentAccessDenied:
            _err = "Content access denied (403)!";
            break;
        default:
            _err = "Unspecified network error!";
    }   // end switch
}   // end _setError

