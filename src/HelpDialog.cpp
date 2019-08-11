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

#include <HelpDialog.h>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextDocument>
#include <QTextStream>
#include <QFile>
#include <iostream>
using QTools::HelpDialog;
using QTools::TreeModel;

class HelpDialog::TreeView : public QTreeView
{
public:
    TreeView( HelpDialog* hd) : QTreeView(hd), _hd(hd) {}

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
    HelpDialog *_hd;
};  // end class


HelpDialog::HelpDialog( QWidget *parent) : QDialog(parent)
{
    if ( parent)
    {
        _wprfx = parent->windowTitle();
        setWindowTitle( _wprfx);
    }   // end if

    setLayout( new QVBoxLayout);

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

    layout()->addWidget( _splitter);

    QPushButton *closeButton = new QPushButton(this);
    closeButton->setText(tr("Close"));
    connect( closeButton, &QPushButton::clicked, this, &HelpDialog::close);
    closeButton->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget( new QWidget);
    hlayout->addWidget( closeButton);

    layout()->addItem(hlayout);

    _tview->setSelectionMode(QAbstractItemView::SingleSelection);
    _tview->setTabKeyNavigation(true);
}   // end ctor


HelpDialog::~HelpDialog()
{
}   // end dtor


QSize HelpDialog::sizeHint() const { return QSize( 900,680);}


void HelpDialog::setSearchPath( const QString& spath)
{
    QStringList spaths;
    spaths << spath;
    _tbrowser->setSearchPaths(spaths);
}   // end setSearchPath


void HelpDialog::setTableOfContents( TreeModel *tm, bool delExisting)
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


void HelpDialog::setContent( const QString& htmlfile)
{
    // Only HTML files defined in the model (table of contents) are allowed.
    QModelIndex idx = static_cast<const TreeModel*>(_tview->model())->find( htmlfile, 1);
    if ( idx.isValid())
    {
        _setContent(htmlfile);
        _tview->setCurrentIndex( idx); // Ensure corresponding entry in TOC is highlighted.
    }   // end if
    else
        std::cerr << "Tried to set HTML content to " << htmlfile.toStdString() << "; something not even in the TOC." << std::endl;
}   // end setContent


void HelpDialog::_setContent( const QString& htmlfile)
{
    QString html;
    QFile f(htmlfile);
    if ( f.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&f);
        html = in.readAll();
    }   // end if

    if ( html.isEmpty())
    {
        std::cerr << "Unable to read in HTML from " << htmlfile.toStdString() << std::endl;
        return;
    }   // end if

    _tbrowser->setHtml( html);

    // Get the title from the HTML's head section.
    QTextDocument doc;
    doc.setHtml( html);
    setWindowTitle( _wprfx + " | " + doc.metaInformation( QTextDocument::MetaInformation::DocumentTitle));
}   // end _setContent
