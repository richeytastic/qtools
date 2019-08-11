/************************************************************************
 * Copyright (C) 2019 Richard Palmer
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

#include <HelpAssistant.h>
#include <TreeModel.h>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QTemporaryFile>
#include <QTextDocument>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
using QTools::HelpAssistant;
using QTools::HelpBrowser;
using QTools::TreeModel;
using QTools::TreeItem;
using PTree = boost::property_tree::ptree;
namespace BFS = boost::filesystem;

namespace {

std::string titleFromHTMLHead( const std::string& htmlfile)
{
    QFile file( QString::fromStdString(htmlfile));
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text))
        return "";
    QTextStream in(&file);
    QString fcontents = in.readAll();
    QTextDocument doc;
    doc.setHtml( fcontents);
    return doc.metaInformation( QTextDocument::MetaInformation::DocumentTitle).toStdString();
}   // end titleFromHTMLHead


bool readSection( const std::string& tdir, const PTree& section, TreeItem* root)
{
    static const std::string istr = " QTools::HelpAssistant::readSection: ";
    std::string fileref = section.get<std::string>("<xmlattr>.ref", "");
    if ( fileref.empty())
    {
        std::cerr << "[WARNING]" << istr << "Section has no ref (file path) attribute!" << std::endl;
        return false;
    }   // end else

    // If the given file reference does not exist, it is skipped.
    const std::string htmlfile = tdir + "/" + fileref;
    if ( !BFS::exists(htmlfile))
    {
        std::cerr << "[WARNING]" << istr << "Skipping section with file path: " << htmlfile << "; it does not exist!" << std::endl;
        return true;
    }   // end if

    // Get if this section specifies a directory to read in arbitrary HTML files placed there.
    const std::string dirref = section.get<std::string>("<xmlattr>.dir", "");
    const std::string dpath = tdir + "/" + dirref;
    // If the directory reference was given but it does not exist, then skip this section.
    if ( !dirref.empty() && !BFS::is_directory(dpath))
    {
        std::cerr << "[WARNING]" << istr << "Skipping section with directory path: " << dpath << "; it does not exist!" << std::endl;
        return true;
    }   // end if

    TreeItem *node = root;

    // Get this section's title (if explicitly defined - otherwise it's obtained from HTML head).
    std::string title = section.get<std::string>("<xmlattr>.title", "");
    if ( title.empty())
        title = titleFromHTMLHead( htmlfile);
    if ( title.empty())
    {
        std::cerr << "[WARNING]" << istr << "Skipping section; title not given explicitly in section, and HTML has no title tag in head!" << std::endl;
        return true;
    }   // end if

    //std::cerr << "Making help section: " << title << " --> " << htmlfile << std::endl;
    node = new TreeItem( {QString::fromStdString(title), QString::fromStdString(htmlfile)}, root);

    // If the section specifies a directory reference then the title must be given and all html files
    // within the directory will be added to the model under it.
    if ( !dirref.empty())
    {
        for ( const auto& dirEnt : BFS::directory_iterator{dpath})
        {
            const BFS::path& path = dirEnt.path();
            const std::string lowRelPathStr = boost::algorithm::to_lower_copy(path.string());
            if ( boost::algorithm::ends_with( lowRelPathStr, ".html"))
            {
                const std::string dhtmlfile = dpath + "/" + path.filename().string();
                //std::cerr << "Got " << dhtmlfile << " from directory" << std::endl;
                const std::string htitle = titleFromHTMLHead( dhtmlfile);
                if ( !htitle.empty())
                    node->appendChild( new TreeItem( {QString::fromStdString(htitle), QString::fromStdString(dhtmlfile)}));
                else
                    std::cerr << "[WARNING]" << istr << " Skipping " << dhtmlfile << "; no title tag given in its head section." << std::endl;
            }   // end if
        }   // end for
    }   // end if

    // Recursively get this section's sections (if it has any).
    for ( const PTree::value_type& v : section)
    {
        if ( v.first != "section")
            continue;

        if ( !readSection( tdir, v.second, node))
            return false;
    }   // end for

    return true;
}   // end readSection


TreeModel* readTableOfContents( const QString& tdir, const QString& tocXMLFile)
{
    QFile tocfile(tocXMLFile);
    if ( !tocfile.open( QIODevice::ReadOnly | QIODevice::Text))
    {
        std::cerr << "[WARNING] QTools::HelpAssistant::readTableOfContents: Unable to read TOC file!" << std::endl;
        return nullptr;
    }   // end if

    QTextStream in(&tocfile);
    QString tocdata = in.readAll();
    tocfile.close();

    std::istringstream iss(tocdata.toStdString());

    PTree tree;
    boost::property_tree::read_xml( iss, tree);

    if ( tree.count("TableOfContents") == 0)
        return nullptr;

    TreeModel *toc = new TreeModel;
    TreeItem *root = toc->setNewRoot({"Table Of Contents"});
    const PTree& xmltoc = tree.get_child("TableOfContents");
    for ( const PTree::value_type& v : xmltoc)
    {
        if ( v.first != "section")
            continue;

        if ( !readSection( tdir.toStdString(), v.second, root))
        {
            delete toc;
            toc = nullptr;
            break;
        }   // end if
    }   // end for

    return toc;
}   // end readTableOfContents

}   // end namespace


HelpAssistant::HelpAssistant( const QString& hdir, QWidget *prnt) : _dialog(new HelpBrowser(prnt))
{
    _initTempHtmlDir(hdir);
    _dialog->setSearchPath( _tdir.path());
}   // end ctor


HelpAssistant::~HelpAssistant()
{
    delete _dialog;
}   // end dtor


bool HelpAssistant::addSubDirectory( const QString& dir)
{
    return QDir(_tdir.path()).mkdir(dir);
}   // end addSubDirectory


void HelpAssistant::_initTempHtmlDir( const QString& srcDir)
{
    QDir sdir(srcDir);
    if ( !sdir.exists())
        return;

    const BFS::path src = sdir.absolutePath().toStdString();
    const BFS::path dst = _tdir.path().toStdString();

    // Copy all content from hdir into _tdir
    for ( const auto& dirEnt : BFS::recursive_directory_iterator{src})
    {
        const auto& path = dirEnt.path();
        auto relPathStr = path.string();
        boost::replace_first( relPathStr, src.string(), "");
        BFS::copy( path, dst / relPathStr);
    }   // end for
}   // end _initTempHtmlDir


QString HelpAssistant::addDocument( const QString& dir, const QString& hfile)
{
    static const std::string werr = "[WARNING] QTools::HelpAssistant::addDocument: ";
    static const std::string istr = "[INFO] QTools::HelpAssistant::addDocument: ";

    QFile file(hfile);  // Input file (resource)
    if ( !file.exists())
    {
        std::cerr << werr << "Input file doesn't exist!" << std::endl;
        return "";
    }   // end if

    BFS::path dst = _tdir.path().toStdString();  // Temporary directory
    dst /= dir.toStdString();   // Subdirectory
    if ( !BFS::exists(dst) || !BFS::is_directory(dst))
    {
        std::cerr << werr << "Invalid directory: '" << dst.string() << "'" << std::endl;
        return "";
    }   // end if

    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text))
    {
        std::cerr << werr << "Unable to read content of file!" << std::endl;
        return "";
    }   // end if

    QTextStream in(&file);
    QString fcontents = in.readAll();

    dst /= "XXXXXX.html"; // Append the filename
    QString tok = QString::fromStdString( dst.string());
    QTemporaryFile tfile( tok);
    tfile.setAutoRemove(false);
    tfile.open();
    tok = tfile.fileName();

    QFile ofile( tok);
    if ( !ofile.open( QIODevice::WriteOnly | QIODevice::Text))
    {
        std::cerr << werr << "Unable to open file for writing!" << std::endl;
        return "";
    }   // end if

    QTextStream out(&ofile);
    out << fcontents;
    ofile.close();

    return tok;
}   // end addDocument


void HelpAssistant::refreshContents( const QString& tocXmlFile)
{
    // Read in the toc.xml file if it exists
    TreeModel *toc = readTableOfContents( _tdir.path(), tocXmlFile);
    _dialog->setTableOfContents(toc);
}   // end refreshContents


bool HelpAssistant::show( const QString& token)
{
    QString hfile = _tdir.path() + "/index.html";
    if ( !token.isEmpty())
        hfile = token;

    _dialog->setContent(hfile);
    _dialog->show();
    _dialog->raise();
    _dialog->activateWindow();
    return true;
}   // end show
