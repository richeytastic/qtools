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

#include <HelpBrowser.h>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextDocument>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <iostream>
using QTools::HelpBrowser;
using QTools::TreeModel;

class HelpBrowser::TreeView : public QTreeView
{
public:
    TreeView( HelpBrowser* hd) : QTreeView(hd), _hd(hd) {}

protected:
    void currentChanged( const QModelIndex& current, const QModelIndex& previous) override
    {
        QTreeView::currentChanged( current, previous);
        if ( current.isValid())
        {
            const QString htmlfile = static_cast<QTools::TreeItem*>(current.internalPointer())->data(1).toString();
            if ( !htmlfile.isEmpty())
                _hd->_setContent(htmlfile);
        }   // end if
    }   // end currentChanged

private:
    HelpBrowser *_hd;
};  // end class

namespace {

QToolButton* makeToolButton( HelpBrowser* d, const QString& iconstr)
{
    QToolButton* tb = new QToolButton(d);
    tb->setIcon( QPixmap( iconstr));
    tb->setMinimumSize( QSize( 36, 26));
    tb->setMaximumSize( QSize( 36, 26));
    tb->setIconSize( QSize( 26, 26));
    tb->setStyleSheet( "QToolButton{ border: none;} \
                        QToolButton:hover{ \
                            border:1px solid #8f8f91; \
                            border-radius: 4px;}");
    return tb;
}   // end makeToolButton

}   // end namespace


HelpBrowser::HelpBrowser( QWidget *parent) : QMainWindow(parent)
{
    if ( parent)
    {
        _wprfx = parent->windowTitle();
        setWindowTitle( _wprfx);
    }   // end if

    QWidget* cwidget = new QWidget;
    cwidget->setLayout( new QVBoxLayout);
    setCentralWidget( cwidget);

    _splitter = new QSplitter(this);
    _tview = new TreeView(this);

    _tbrowser = new QTextBrowser;
    _splitter->addWidget( _tview);
    _splitter->addWidget( _tbrowser);

    QSizePolicy policy = _tview->sizePolicy();
    policy.setHorizontalStretch(1);
    _tview->setSizePolicy(policy);

    policy = _tbrowser->sizePolicy();
    policy.setHorizontalStretch(4);
    _tbrowser->setSizePolicy(policy);

    _splitter->setCollapsible(0,false);
    _splitter->setCollapsible(1,false);

    cwidget->layout()->addWidget( _splitter);

    QPushButton *closeButton = new QPushButton(this);
    closeButton->setText(tr("Close"));
    connect( closeButton, &QPushButton::clicked, this, &HelpBrowser::close);
    closeButton->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum);

    QToolButton *homeButton = makeToolButton( this, ":/icons/HOME");
    connect( homeButton, &QToolButton::clicked, [this](){ _tbrowser->home();});

    _backButton = makeToolButton( this, ":/icons/GO_BACK");
    connect( _backButton, &QToolButton::clicked, [this](){ _tbrowser->backward();});

    _fwrdButton = makeToolButton( this, ":/icons/GO_FWRD");
    connect( _fwrdButton, &QToolButton::clicked, [this](){ _tbrowser->forward();});

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget( _backButton);
    hlayout->addWidget( _fwrdButton);
    hlayout->addWidget( homeButton);
    hlayout->insertStretch(3);
    hlayout->addWidget( closeButton);

    cwidget->layout()->addItem(hlayout);

    _tview->setSelectionMode(QAbstractItemView::SingleSelection);
    _tbrowser->setOpenExternalLinks(true);

    connect( _tbrowser, &QTextBrowser::sourceChanged, this, &HelpBrowser::_doOnSourceChanged);
}   // end ctor


HelpBrowser::~HelpBrowser()
{
}   // end dtor


QSize HelpBrowser::sizeHint() const { return QSize( 1024, 680);}


void HelpBrowser::setSearchPath( const QString& spath)
{
    QStringList spaths;
    spaths << spath;
    _tbrowser->setSearchPaths(spaths);
}   // end setSearchPath


void HelpBrowser::setTableOfContents( TreeModel *tm, bool delExisting)
{
    QItemSelectionModel *delsm = _tview->selectionModel();   // To delete if not null
    QAbstractItemModel *delm = _tview->model();

    _tview->setModel( tm);
    _tview->expandAll();

    if ( delm && delExisting)
    {
        delete delm;
        if ( delsm)
            delete delsm;
    }   // end if
}   // end setTableOfContents


bool HelpBrowser::setContent( const QString& htmlfile)
{
    bool ok = false;
    if ( _updateTOCIndex(htmlfile))
        ok = _setContent(htmlfile);
    return ok;
}   // end setContent


bool HelpBrowser::_setContent( const QString& htmlfile)
{
    _tbrowser->setSource( htmlfile);
    _updateTitleFromContent();
    _backButton->setEnabled( _tbrowser->isBackwardAvailable());
    _fwrdButton->setEnabled( _tbrowser->isForwardAvailable());
    return true;
}   // end _setContent


void HelpBrowser::_doOnSourceChanged( const QUrl &src)
{
    if ( !_updateTOCIndex( src.path()))
        _updateTOCIndex( src.path());
    _updateTitleFromContent();
}   // end _doOnSourceChanged


void HelpBrowser::_updateTitleFromContent()
{
    // Get the title from the HTML's head section.
    QTextDocument doc;
    doc.setHtml( _tbrowser->toHtml());
    setWindowTitle( _wprfx + " | " + doc.metaInformation( QTextDocument::MetaInformation::DocumentTitle));
}   // end _updateTitleFromContent


bool HelpBrowser::_updateTOCIndex( const QString& htmlfile)
{
    // Only HTML files defined in the model (table of contents) are allowed.
    QModelIndex idx = static_cast<const TreeModel*>(_tview->model())->find( htmlfile, 1);
    if ( idx.isValid())
        _tview->setCurrentIndex( idx); // Ensure corresponding entry in TOC is highlighted.
    return idx.isValid();
}   // end _updateTOCIndex



