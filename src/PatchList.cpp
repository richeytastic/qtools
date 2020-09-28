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

#include <QTools/PatchList.h>
#include <rlib/StringUtil.h>
#include <boost/property_tree/xml_parser.hpp>
#include <QSet>
#include <iostream>
using PTree = boost::property_tree::ptree;
using QTools::PatchList;
using QTools::PatchMeta;
using QTools::PatchFiles;


/************************************/
/************ PatchList *************/
/************************************/

PatchList::PatchList() {}


bool PatchList::setCurrentVersion( int mj, int mn, int pt)
{
    PatchMeta nver( mj, mn, pt);
    if ( nver >= _currv)
    {
        _appName = _appTgtDir = _err = "";
        _patches.clear();
        _currv = nver;
    }   // end if
    return _currv == nver;
}   // end setCurrentVersion


bool PatchList::setCurrentVersion( const PatchMeta &pm)
{
    return setCurrentVersion( pm.major(), pm.minor(), pm.patch());
}   // end setCurrentVersion


const PatchMeta& PatchList::highestVersion() const
{
    if ( _patches.isEmpty())
        return _currv;
    return _patches.first();
}   // end highestVersion


QString PatchList::patchDescription() const
{
    QStringList desc;
    for ( const PatchMeta &pm : _patches)
        desc.append( pm.description());
    return desc.join("\n");
}   // end patchDescription


QList<QUrl> PatchList::patchURLs() const
{
    QList<QUrl> purls;
    for ( const PatchMeta &pm : _patches)
        purls.push_back( pm.patchUrl());
    return purls;
}   // end patchURLs


bool PatchList::parse( const QByteArray &xmldata)
{
    _err = "";
    std::istringstream iss( xmldata.toStdString());
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

    if ( tree.count("PatchList") == 0)
    {
        _err = "PatchList tag not found!";
        return false;
    }   // end if

    const PTree &plist = tree.get_child("PatchList");
    if ( plist.count("Application") == 0)
    {
        _err = "Application name not found!";
        return false;
    }   // end if

    if ( plist.count("TargetDir") == 0)
    {
        _err = "Application target directory not found!";
        return false;
    }   // end if

    if ( plist.count("Patches") == 0)
    {
        _err = "Patches tag not found!";
        return false;
    }   // end if

    _appName = QString::fromStdString( rlib::trim( plist.get<std::string>( "Application")));
    if ( _appName.isEmpty())
    {
        _err = "Empty Application name!";
        return false;
    }   // end if

    _appTgtDir = QString::fromStdString( rlib::trim( plist.get<std::string>( "TargetDir")));
    if ( _appTgtDir.isEmpty())
    {
        _err = "Empty Application target directory!";
        return false;
    }   // end if

    const PTree &ptree = plist.get_child("Patches");
    for ( const PTree::value_type &pval : ptree)
        if ( !_parsePatchMeta( pval.second))
            break;

    // Sort in descending order so the most recent (highest) version is first.
    std::sort( _patches.rbegin(), _patches.rend());

    // Now cull the list of patches based on the files they include (only care about
    // the latest version of a given file).
    _cullForDuplicateFiles();

    return _err.isEmpty();
}   // end parse


void PatchList::_cullForDuplicateFiles()
{
    QSet<QString> fileset;
    QList<PatchMeta> npatches;
    for ( const PatchMeta &pm : _patches)
    {
        const int fsize = fileset.size();
        const PatchFiles &pfiles = pm.files();
        for ( const QString &file : pfiles.files())
            fileset.insert(file);
        // This patch is used if there's at least one file not
        // in the existing fileset from more recent patches.
        if ( fileset.size() > fsize)
        {
            std::cerr << "Using patch " << pm.major() << "." << pm.minor() << "." << pm.patch() << std::endl;
            npatches.push_back(pm);
        }   // end if
    }   // end for
    _patches = npatches;   // Replace
}   // end _cullForDuplicateFiles


namespace {
int getVersionAttr( const PTree &pnode, const std::string &vstr)
{
    boost::optional<std::string> v = pnode.get_optional<std::string>(vstr);
    int ival = -1;
    if ( v)
    {
        bool okay = false;
        ival = QString::fromStdString( *v).toInt( &okay);
        if ( !okay)
            ival = -1;
    }   // end if
    return ival;
}   // end getVersionAttr
}   // end namespace


bool PatchList::_parsePatchMeta( const PTree &pnode)
{
    if ( pnode.count("Description") == 0)
    {
        _err = "Missing Description in Patch!";
        return false;
    }   // end if

    if ( pnode.count("BaseURL") == 0)
    {
        _err = "Missing BaseURL in Patch!";
        return false;
    }   // end if

    if ( pnode.count("Platforms") == 0)
    {
        _err = "Missing Platforms in Patch!";
        return false;
    }   // end if

    PatchMeta meta;

    if ( !meta.setMajor( getVersionAttr( pnode,"<xmlattr>.major"))
      || !meta.setMinor( getVersionAttr( pnode,"<xmlattr>.minor"))
      || !meta.setPatch( getVersionAttr( pnode,"<xmlattr>.patch")))
    {
        _err = "Missing version in Patch!";
        return false;
    }   // end if

    // If the parsed patch version is <= than current, just return true.
    if ( meta <= _currv)
    {
        std::cerr << QString("Ignoring patch %1.%2.%3")
                .arg(meta.major()).arg(meta.minor()).arg(meta.patch()).toStdString() << std::endl;
        return true;
    }   // end if

    if ( !meta.setDescription( QString::fromStdString( rlib::trim( pnode.get<std::string>("Description")))))
    {
        _err = "Empty Description in Patch!";
        return false;
    }   // end if

    if ( !meta.setBaseUrl( QString::fromStdString( rlib::trim( pnode.get<std::string>("BaseURL")))))
    {
        _err = "Empty BaseURL in Patch!";
        return false;
    }   // end if

    if ( !meta.isValid())
    {
        _err = "Invalid Patch!";
        return false;
    }   // end if

    // Get the patch manifest for each supported platform
    const PTree &platforms = pnode.get_child("Platforms");
    for ( const PTree::value_type &pval : platforms)
    {
        // If added the platform, break since we don't need other platforms.
        if ( _parsePatchFiles( meta, pval.second))
            break;
        // Also break if an error set
        if ( !_err.isEmpty())
            break;
    }   // end for

    if ( _err.isEmpty())
        _patches.push_back(meta);

    return _err.isEmpty();
}   // end _parsePatchMeta


bool PatchList::_parsePatchFiles( PatchMeta &meta, const PTree &pnode)
{
    boost::optional<std::string> pname = pnode.get_optional<std::string>("<xmlattr>.name");
    if ( !pname)
    {
        _err = "Missing name attribute in Platform!";
        return false;
    }   // end if

    // Only want the patch manifest for this platform
    std::string thisPlatform;
#ifdef _WIN32
    thisPlatform = "Windows";
#elif __linux__
    thisPlatform = "Linux";
#endif

    if ( rlib::trim( (const std::string)(*pname)) != thisPlatform)
        return false;

    if ( pnode.count("Archive") == 0)
    {
        _err = "Missing Archive tag in Platform!";
        return false;
    }   // end if

    if ( pnode.count("Modify") == 0)
    {
        _err = "Missing Modify tag in Platform!";
        return false;
    }   // end if

    PatchFiles pfiles;

    // Set the name of the archive file on the server.
    if ( !pfiles.setArchive( QString::fromStdString( rlib::trim( pnode.get<std::string>("Archive")))))
    {
        _err = "Invalid Archive in Platform!";
        return false;
    }   // end if

    const PTree &mnode = pnode.get_child("Modify");
    for ( const PTree::value_type &fval : mnode)
    {
        const std::string fname = rlib::trim( fval.second.get_value<std::string>());
        if ( !pfiles.addFile( QString::fromStdString( fname)))
        {
            _err = "Invalid File in Platform!";
            break;
        }   // end if
    }   // end for

    if ( pfiles.files().isEmpty() && _err.isEmpty())
        _err = "No files given in patch manifest!";

    if ( _err.isEmpty())
        meta.setFiles( pfiles);

    return _err.isEmpty();
}   // end _parsePatchFiles


/*************************************/
/************ PatchFiles *************/
/*************************************/

PatchFiles::PatchFiles() {}


bool PatchFiles::setArchive( const QString &v)
{
    _archive = v;
    return !_archive.isEmpty();
}   // end setArchive


bool PatchFiles::addFile( const QString &f)
{
    if ( f.isEmpty())
        return false;
    _files.push_back(f);
    return true;
}   // end addFile


/************************************/
/************ PatchMeta *************/
/************************************/

PatchMeta::PatchMeta( int mj, int mn, int pt)
    : _major(0), _minor(0), _patch(0)
{
    setMajor( mj);
    setMinor( mn);
    setPatch( pt);
}   // end ctor


bool PatchMeta::isValid() const
{
    return (_major > 0 || _minor > 0 || _patch > 0) && !_baseUrl.isEmpty();
}   // end isValid


bool PatchMeta::operator<( const PatchMeta &v) const
{
    const int mj = v.major();
    const int mn = v.minor();
    const int pt = v.patch();
    return (mj  > _major)
        || (mj == _major && mn  > _minor)
        || (mj == _major && mn == _minor && pt > _patch);
}   // end operator<


bool PatchMeta::operator==( const PatchMeta &v) const
{
    return v.major() == _major && v.minor() == _minor && v.patch() == _patch;
}   // end operator==

bool PatchMeta::operator>=( const PatchMeta &v) const { return !(*this < v);}

bool PatchMeta::operator!=( const PatchMeta &v) const { return !(*this == v);}

bool PatchMeta::operator>( const PatchMeta &v) const { return (*this >= v) && (*this != v);}

bool PatchMeta::operator<=( const PatchMeta &v) const { return !(*this > v);}


bool PatchMeta::setMajor( int v)
{
    _major = std::max( _major, v);
    return _major == v;
}   // end setMajor


bool PatchMeta::setMinor( int v)
{
    _minor = std::max( _minor, v);
    return _minor == v;
}   // end setMinor


bool PatchMeta::setPatch( int v)
{
    _patch = std::max( _patch, v);
    return _patch == v;
}   // end setPatch


bool PatchMeta::setDescription( const QString &v)
{
    _deets = v;
    return !_deets.isEmpty();
}   // end setDescription


bool PatchMeta::setBaseUrl( const QString &v)
{
    _baseUrl = v;
    return !_baseUrl.isEmpty();
}   // end setBaseUrl


QUrl PatchMeta::patchUrl() const { return QUrl( baseUrl() + "/" + _platform.archive());}
