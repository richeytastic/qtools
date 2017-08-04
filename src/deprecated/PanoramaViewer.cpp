/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

#include "PanoramaViewer.h"
using QTools::PanoramaViewer;
#include "ui_PanoramaViewer.h"
#include "QImageTools.h"
#include <QVBoxLayout>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <cassert>
#include <iostream>
using std::cerr;
using std::endl;



void PanoramaViewer::displayKeyboardCommands( ostream& os)
{
    os << "Select image mode type:" << std::endl;
    os << " 1 -> Colour image" << std::endl;
    os << " 2 -> Depth map (contrast normalised)" << std::endl;
    os << " 3 -> Point cloud (use GUI to switch back to other image modes)" << std::endl;
    os << "View face changing:" << std::endl;
    os << " q -> Rotate view to left (colour or depth image)" << std::endl;
    os << " e -> Rotate view to right (colour or depth image)" << std::endl;
    os << "Feature extraction (use mouse to click and drag new rectangles):" << std::endl;
    os << " w -> Move rectangle up 1 pixel" << std::endl;
    os << " s -> Move rectangle down 1 pixel" << std::endl;
    os << " a -> Move rectangle left 1 pixel" << std::endl;
    os << " d -> Move rectangle right 1 pixel" << std::endl;
    os << " shift + [w,s,a,d] -> Resize rectangle 1 pixel from bottom/right edge" << std::endl;
    os << " x -> Extract current feature from rectangle" << std::endl;
}   // end displayKeyboardCommands


PanoramaViewer::PanoramaViewer( const string &gtDir, QWidget *parent) : QWidget(parent), _ui(new Ui::PanoramaViewer)
{
    _ui->setupUi(this);
    this->installEventFilter(this); // For keyboard shortcuts

    _viewFrame = new QTools::ViewFrame( this);
    connect( _viewFrame, SIGNAL( showingUserRectangle(cv::Rect)), this, SLOT( showingUserRectangleSlot(cv::Rect)));
    connect( _viewFrame, SIGNAL( extractFeature(cv::Rect)), this, SLOT( extractFeature()));
    connect( _viewFrame, SIGNAL( eraseFeature(cv::Rect)), this, SLOT( eraseFeature(cv::Rect)));
    connect( _viewFrame, SIGNAL( extractImage(cv::Rect)), this, SLOT( extractImage(cv::Rect)));

    // View navigation
    _panoNav = new QTools::ViewNavigator(this);
    connect( _panoNav, SIGNAL( selectedView( ViewNavigator::ViewFace)), this, SLOT( showView( ViewNavigator::ViewFace)));
    connect( _panoNav, SIGNAL( selectedImage()), this, SLOT( changeViewImage()));
    connect( _panoNav, SIGNAL( selectedDepth()), this, SLOT( changeViewImage()));
    connect( _panoNav, SIGNAL( selectedPoints()), this, SLOT( changeViewImage()));

    // Add to layout
    _ui->viewFrame->layout()->addWidget( _viewFrame);
    _ui->navigatorFrame->layout()->addWidget( _panoNav);

    connect( _ui->createFeatureClassButton, SIGNAL( clicked()), this, SLOT( createNewFeatureClass()));
    connect( _ui->featureClassComboBox, SIGNAL( activated(int)), this, SLOT( selectFeatureClass()));
    connect( _ui->extractFeatureButton, SIGNAL( clicked()), this, SLOT( extractFeature()));

    _featureWriter = new FeatureWriter( gtDir);
    populateFeatureClassComboBox();
    reset();
}   // end ctor



PanoramaViewer::~PanoramaViewer()
{
    reset();
    delete _featureWriter;
    delete _panoNav;
    delete _viewFrame;
    delete _ui;
}   // end dtor



void PanoramaViewer::setGroundTruthDir(const string &gtDir)
{
    delete _featureWriter;
    _featureWriter = new FeatureWriter( gtDir);
    populateFeatureClassComboBox();
}   // end setGroundTruthDir



cv::Mat PanoramaViewer::getViewImage() const
{
    View::Ptr v = getCurrentView();
    assert( v != NULL);
    return v->img2d.clone();
}   // end getViewImage


View::Ptr PanoramaViewer::getCurrentView() const
{
    return getView( getCurrentViewFace());
}   // end getCurrentView


ViewNavigator::ViewFace PanoramaViewer::getCurrentViewFace() const
{
    return _panoNav->getViewDirection();
}   // end getCurrentViewFace


bool PanoramaViewer::gotPoints() const
{
    if ( _pano != NULL)
        return true;
    return false;
}   // end gotPoints


void PanoramaViewer::setTempNonViewImage( const cv::Mat &img)
{
    _viewFrame->setImage( img);
}   // end setTempNonViewImage


void PanoramaViewer::resetViewImage()
{
    View::Ptr view = getCurrentView();
    if ( view != NULL)
        _viewFrame->setImage( view->img2d);
}   // end resetViewImage


void PanoramaViewer::setModelSize( const cv::Size2f &modSize)
{
    _viewFrame->setModelSize(modSize);
}   // end setModelSize



// public slot
void PanoramaViewer::setPanorama( const Panorama::Ptr pn, const string& dataId)
{
    reset();
    _pano = pn;
    _viewFrame->addPoints( pn->getFrontView()->points, pn->getFrontView()->img2d, 0.25);
    _viewFrame->addPoints( PointCloudOrienter( pn->getLeftView()->points).copyToLeft(), pn->getLeftView()->img2d, 0.5);
    _viewFrame->addPoints( PointCloudOrienter( pn->getRearView()->points).copyToRear(), pn->getRearView()->img2d, 0.75);
    _viewFrame->addPoints( PointCloudOrienter( pn->getRightView()->points).copyToRight(), pn->getRightView()->img2d, 1.0);
    _panoNav->enableControls(true);
    showView( getCurrentViewFace());

    updateFeatureWriter();
    _featureWriter->setDataId( dataId);
    _dataId = dataId;
    selectFeatureClass();
}   // end setPanorama



// public slot
void PanoramaViewer::setImage( const cv::Mat& img, const string& dataId)
{
    reset();
    _viewFrame->setImage( img);
    _panoNav->enableControls(false);
    updateFeatureWriter();
    _featureWriter->setDataId( dataId);
    _dataId = dataId;
    selectFeatureClass();
}   // end setImage



// private slot
void PanoramaViewer::showView( ViewNavigator::ViewFace face)
{
    // Reset the point cloud view to the front for this view
    View::Ptr v = getView(face);
    _viewFrame->showView( v, face);
    changeViewImage();
    emit viewChanged( face);
}   // end showView



// private slot
void PanoramaViewer::changeViewImage()
{
    const ViewNavigator::ViewType vt = _panoNav->getViewType();
    if ( vt == ViewNavigator::IMAGE)
        _viewFrame->viewImage();
    else if ( vt == ViewNavigator::DEPTH)
        _viewFrame->viewDepth();
    else if ( vt == ViewNavigator::POINTS)
        _viewFrame->viewPoints();
    selectFeatureClass();
}   // end changeViewImage



// private slot
void PanoramaViewer::reset()
{
    _panoNav->reset();
    _viewFrame->reset();
    _pano.reset();
    _dataId = "";
}   // end reset



// Displays bounding boxes of features from the current class on the view
// private slot
void PanoramaViewer::selectFeatureClass()
{
    updateFeatureWriter();

    // Find all matching records for the current pano
    vector<RFeatures::FeatureRecord> recs;
    _featureWriter->findRecords( _featureWriter->getDataId(), recs);

    // Display the bounding boxes of these examples as thick blue on the view
    _viewFrame->clearFeatureRectangles();   // (Clear existing)
    foreach ( const RFeatures::FeatureRecord &f, recs)
    {
        const ViewNavigator::ViewFace featFace = (ViewFace)QString( f.viewInfo.c_str()).toInt();
        const ViewNavigator::ViewFace vface = _panoNav->getViewDirection();
        if ( vface == featFace)
            _viewFrame->showFeatureRectangle( f.boundingBox);
    }   // end foreach

    //this->setFocus();
}   // end selectFeatureClass



// private slot
void PanoramaViewer::createNewFeatureClass()
{
    bool ok;
    QString cname = QInputDialog::getText(this, "Enter Class Name", "Feature Class:", QLineEdit::Normal, "", &ok);
    if ( !ok) return;

    // Check if cname is a duplicate of an existing class
    if ( _featureWriter->classExists( cname.toStdString()))
    {
        QMessageBox::information(this, "Feature class exists",
                    "Class name already exists! Choose another or use the existing if appropriate.");
        return;
    }   // end if

    _ui->featureClassComboBox->addItem( cname);
    // Set selected class to the newly added class
    _ui->featureClassComboBox->setCurrentIndex( _ui->featureClassComboBox->findText( cname));
    selectFeatureClass();

    //this->setFocus();
}   // end createNewFeatureClass



// public slot
void PanoramaViewer::enableNavigation(bool enabled)
{
    _panoNav->enableControls(enabled);
}   // end enableNavigation



// private
void PanoramaViewer::populateFeatureClassComboBox()
{
    enableFeatureExtract(false);
    vector<string> cnames;
    _featureWriter->getExistingClassNames( cnames);

    _ui->featureClassComboBox->setInsertPolicy(QComboBox::InsertAlphabetically);
    QStringList cnamesList;
    foreach( const string &cn, cnames)
        cnamesList.append( QString( cn.c_str()));
    _ui->featureClassComboBox->addItems( cnamesList);

    if ( _ui->featureClassComboBox->count() > 0)
    {
        _ui->featureClassComboBox->setCurrentIndex(0);
        selectFeatureClass();
    }   // end if
}   // end populateFeatureClassComboBox



// private
void PanoramaViewer::updateFeatureWriter()
{
    const ViewNavigator::ViewFace face = _panoNav->getViewDirection();
    std::ostringstream oss2;
    oss2 << (int)face;
    _featureWriter->setViewInfo( oss2.str());

    // Update FeatureWriter and hand off to the viewer module
    const QString cname = _ui->featureClassComboBox->currentText();
    if ( !cname.isEmpty())
    {
        const string className = cname.toLower().toStdString();
        _featureWriter->setClass( className);
    }   // end if
}   // end updateFeatureWriter



// private
View::Ptr PanoramaViewer::getView( ViewNavigator::ViewFace face) const
{
    View::Ptr view;

    if ( _pano != NULL)
    {
        switch ( face)
        {
            case ViewNavigator::FRONT:
                view = _pano->getFrontView();
                break;
            case ViewNavigator::LEFT:
                view = _pano->getLeftView();
                break;
            case ViewNavigator::REAR:
                view = _pano->getRearView();
                break;
            case ViewNavigator::RIGHT:
                view = _pano->getRightView();
                break;
        }   // end switch
    }   // end if

    return view;
}   // end getView



// private slot
void PanoramaViewer::showingUserRectangleSlot( const cv::Rect rct)
{
    // If user has described a rectangle, and we have classes to extract for, and we have some data set, enable the extractor.
    enableFeatureExtract(rct.area() > 0 && _ui->featureClassComboBox->count() > 0 && !_dataId.empty());
    emit showingUserRectangle( rct);
}   // end showingUserRectangleSlot



// private slot
void PanoramaViewer::extractImage( const cv::Rect r) const
{
    const cv::Mat img = getViewImage();
    const cv::Mat imgEx = img(r);
    QImageTools::saveImage( imgEx);
}   // end extractImage



// private slot
void PanoramaViewer::extractFeature()
{
    const cv::Rect r = _viewFrame->getSelectedRectangle();
    if ( r.area() == 0)
    {
        std::cerr << "No feature defined!" << std::endl;
        return;
    }   // end if

    const int fid = _featureWriter->recordInstance(r);
    const string className = _featureWriter->getClass();
    std::cout << "Feature " << fid << " (" << className << ") recorded" << std::endl;
    _viewFrame->showFeatureRectangle( r);
    //this->setFocus();
}   // end extractFeature


// private slot
void PanoramaViewer::eraseFeature( const cv::Rect rct) const
{
    const string viewString = ViewNavigator::viewFaceToString( this->getCurrentViewFace());
    const string dataId = _featureWriter->getDataId();
    const int eraseCount = _featureWriter->eraseInstance( rct, dataId, viewString);
    const string className = _featureWriter->getClass();
    std::cerr << "Erased " << eraseCount << " feature from class " << className
              << " from panorama " << dataId << "(" << viewString << ")" << std::endl;
    if ( eraseCount > 0)
        _viewFrame->clearFeatureRectangle(rct);
    else
        std::cerr << "ERROR: Corresponding feature for parameter rectangle not present in FeatureWriter!" << std::endl;

    if ( eraseCount > 1)
        std::cerr << "WARNING (Debug): Multiple identical features were erased! No action is needed." << std::endl;
}   // end eraseFeature



// private slot
void PanoramaViewer::enableFeatureExtract( bool enable)
{
    _ui->extractFeatureButton->setEnabled(enable);
    _viewFrame->enableFeatureExtract(enable);
}   // end enableFeatureExtract



void PanoramaViewer::keyPressEvent( QKeyEvent* evt)
{
    if ( _pano == NULL)
        return;

    const bool shiftMod = evt->modifiers() & Qt::ShiftModifier;
    const int key = evt->key();

    int xDelta = 0, yDelta = 0;
    int horzDelta = 0, vertDelta = 0;
    int vd = 0;
    switch ( key)
    {
        case Qt::Key_1:
            _panoNav->setViewType( ViewNavigator::IMAGE);
            break;
        case Qt::Key_2:
            _panoNav->setViewType( ViewNavigator::DEPTH);
            break;
        case Qt::Key_3:
            _panoNav->setViewType( ViewNavigator::POINTS);
            break;
        case Qt::Key_W: // Reduction - Y
            if ( shiftMod) vertDelta = -1;
            else yDelta = -1;
            break;
        case Qt::Key_S: // Expansion - Y
            if ( shiftMod) vertDelta = 1;
            else yDelta = 1;
            break;
        case Qt::Key_A: // Reduction - X
            if ( shiftMod) horzDelta = -1;
            else xDelta = -1;
            break;
        case Qt::Key_D: // Expansion - X
            if ( shiftMod) horzDelta = 1;
            else xDelta = 1;
            break;
        case Qt::Key_Q: // Rotate view left
            vd = int(_panoNav->getViewDirection()) + 1;
            if ( vd == 4) vd = 0;
            _panoNav->setViewDirection( ViewNavigator::ViewFace(vd));
            this->showView( _panoNav->getViewDirection());
            break;
        case Qt::Key_E: // Rotate view right
            vd = int(_panoNav->getViewDirection()) - 1;
            if ( vd < 0) vd = 3;
            _panoNav->setViewDirection( ViewNavigator::ViewFace(vd));
            this->showView( _panoNav->getViewDirection());
            break;
        case Qt::Key_X:   // Extract user rectangle (if present)
            extractFeature();
            break;
        default:    // No-op
            break;
    }   // end switch

    if ( key == Qt::Key_W || key == Qt::Key_S || key == Qt::Key_A || key == Qt::Key_D)
    {
        _viewFrame->shiftUserRectangle( xDelta, yDelta);
        _viewFrame->resizeUserRectangle( horzDelta, vertDelta);
        evt->accept();
    }   // end if
    else if ( key == Qt::Key_1 || key == Qt::Key_2)// || key == Qt::Key_3)
    {
        this->changeViewImage();
        evt->accept();
    }   // end else if

    this->setFocus();
}   // end keyPressEvent
