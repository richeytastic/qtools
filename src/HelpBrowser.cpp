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

#include <HelpBrowser.h>
#include <QGuiApplication>
#include <QSignalBlocker>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextDocument>
#include <QTextStream>
#include <QScreen>
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
    void currentChanged( const QModelIndex& crnt, const QModelIndex& prvs) override
    {
        QTreeView::currentChanged( crnt, prvs);
        QString htmlfile;
        if ( crnt.isValid() && crnt.internalPointer() != prvs.internalPointer())
            htmlfile = static_cast<QTools::TreeItem*>(crnt.internalPointer())->data(1).toString();
        if ( !htmlfile.isEmpty())
            _hd->_setContent(htmlfile);
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
    _tbrowser->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn);

    connect( _tbrowser, &QTextBrowser::sourceChanged, this, &HelpBrowser::_doOnSourceChanged);

    setGeometry( QStyle::alignedRect( Qt::LeftToRight, Qt::AlignCenter, sizeHint(), QGuiApplication::primaryScreen()->geometry()));
}   // end ctor


void HelpBrowser::setRootDir( const QString& rdir)
{
    _rootDir = rdir + "/";
}   // end setRootDir


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
    // Only HTML files defined in the model (table of contents) are allowed.
    const QModelIndex idx = static_cast<const TreeModel*>(_tview->model())->find( htmlfile, 1);
    if ( idx.isValid())
    {
        _tview->setCurrentIndex( idx); // Ensure corresponding entry in TOC is highlighted.
        ok = true;
    }   // end if
    //std::cerr << "setContent( " << htmlfile.toStdString() << ") --> " << std::boolalpha << ok << std::endl;
    return ok;
}   // end setContent


void HelpBrowser::_setContent( const QString& htmlfile)
{
    QSignalBlocker blocker(_tbrowser);
    _tbrowser->setSource( QUrl::fromLocalFile( _rootDir + htmlfile), QTextDocument::HtmlResource);
    setWindowTitle( _wprfx + " | " + _tbrowser->documentTitle());
    _backButton->setEnabled( _tbrowser->isBackwardAvailable());
    _fwrdButton->setEnabled( _tbrowser->isForwardAvailable());
    //std::cerr << "_setContent( " << htmlfile.toStdString() << ")" << std::endl;
}   // end _setContent


// This function called if links clicked in the page
void HelpBrowser::_doOnSourceChanged( const QUrl &src)
{
    QString path = QDir::fromNativeSeparators( src.path());
    path.remove(_rootDir);
    if ( path.startsWith('/'))
        path = path.right( path.size()-1);
    const QModelIndex idx = static_cast<const TreeModel*>(_tview->model())->find( path, 1);
    if ( idx.isValid())
        _tview->setCurrentIndex( idx);  // Calls _setContent through _tview->currentChanged
    else
        std::cerr << "[ERROR] QTools::HelpBrowser::_doOnSourceChanged: Invalid \"" << path.toStdString() << "\"\n";
}   // end _doOnSourceChanged
