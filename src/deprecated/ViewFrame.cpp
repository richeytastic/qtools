#include "ViewFrame.h"
using QTools::ViewFrame;
#include "ui_ViewFrame.h"
#include <cassert>
#include <FeatureUtils.h>   // RFeatures


ViewFrame::ViewFrame(QWidget *parent) :
    QFrame(parent), ui(new Ui::ViewFrame)
{
    ui->setupUi(this);

    // Create the image viewer and its context menu
    imageViewer_ = new QTools::ImagerWidget(this);

    connect( imageViewer_, SIGNAL( showingUserRectangle( cv::Rect)), this, SIGNAL( showingUserRectangle( cv::Rect)));
    connect( imageViewer_, SIGNAL( extractImage( cv::Rect)), this, SIGNAL( extractImage( cv::Rect)));
    connect( imageViewer_, SIGNAL( extractFeature( cv::Rect)), this, SIGNAL( extractFeature( cv::Rect)));
    connect( imageViewer_, SIGNAL( eraseFeature( cv::Rect)), this, SIGNAL( eraseFeature( cv::Rect)));

    pcloudViewer_ = new QTools::PointCloudViewer(this);

    ui->viewFrame->layout()->addWidget( imageViewer_);
    ui->viewFrame->layout()->addWidget( pcloudViewer_);

    pcloudViewer_->hide();
}   // end ctor


ViewFrame::~ViewFrame()
{
    delete pcloudViewer_;
    delete imageViewer_;
    delete ui;
}   // end dtor



cv::Rect ViewFrame::getSelectedRectangle() const
{
    return imageViewer_->getSelectedRectangle();
}   // end getSelectedRectangle



// public slot
void ViewFrame::showFeatureRectangle( const cv::Rect &rct)
{
    imageViewer_->addRectangle( rct);
}   // end showFeatureRectangle


// public slot
void ViewFrame::clearFeatureRectangle(const cv::Rect &rct)
{
    imageViewer_->removeRectangle(rct);
}   // end clearFeatureRectangle


// public slot
void ViewFrame::clearFeatureRectangles()
{
    imageViewer_->clearRectangles();
}   // end clearFeatureRectangles


// public slot
void ViewFrame::enableFeatureExtract(bool enable)
{
    imageViewer_->enableFeatureExtract(enable);
}   // end enableFeatureExtract


// public slot
void ViewFrame::shiftUserRectangle( int xDelta, int yDelta)
{
    imageViewer_->shiftUserRectangle( xDelta, yDelta);
}   // end shiftUserRectangle


// public slot
void ViewFrame::resizeUserRectangle( int horzDelta, int vertDelta)
{
    imageViewer_->resizeUserRectangle( horzDelta, vertDelta);
}   // end resizeUserRectangle


// public slot
void ViewFrame::viewImage()
{
    pcloudViewer_->hide();
    imageViewer_->show();
    if ( view_ != 0)
    {
        imageViewer_->setImage( view_->img2d);
        imageViewer_->refresh();
    }   // end if
}   // end viewImage


// public slot
void ViewFrame::viewDepth()
{
    pcloudViewer_->hide();
    imageViewer_->show();
    if ( view_ != 0)
    {
        const cv::Mat_<byte> dimg = RFeatures::makeDisplayableRangeMap( view_->rngImg);
        imageViewer_->setImage( dimg);
        imageViewer_->refresh();
    }   // end if
}   // end viewDepth


// public slot
void ViewFrame::viewPoints()
{
    imageViewer_->hide();
    pcloudViewer_->show();
}   // end viewPoints


// public slot
void ViewFrame::reset()
{
    totPoints_ = 0;
    pcloudViewer_->clear();
    pcloudViewer_->setProgressBar(0,0);
    const cv::Vec3d zeroVec(0,0,0);
    pcloudViewer_->resetPointCloud( zeroVec, zeroVec, zeroVec);
    pcloudViewer_->updateRender();
    imageViewer_->clearImage();
    imageViewer_->clearRectangles();
    viewImage();
}   // end reset


void ViewFrame::setImage( const cv::Mat &img)
{
    imageViewer_->setImage(img);
}   // end setImage


cv::Mat ViewFrame::getImage() const
{
    return imageViewer_->getImage();
}   // end getImage


void ViewFrame::setModelSize( const cv::Size2f &modSz)
{
    imageViewer_->setModelSize( modSz);
}   // end setModelSize


void ViewFrame::addPoints( const cv::Mat_<cv::Vec3f> pc, const cv::Mat_<cv::Vec3b> ci, double prop)
{
    totPoints_ += (int)pc.total();
    pcloudViewer_->addPoints( pc, ci);
    imageViewer_->refresh();
    pcloudViewer_->setProgressBar( (int)(100 * prop + 0.5), totPoints_);
}   // end addPoints


// private
cv::Vec3d ViewFrame::getFaceFocus( ViewFace face)
{
    static const double FOC_LEN = 1e-8;
    cv::Vec3d foc;
    switch ( face)
    {
        case QTools::ViewNavigator::FRONT:
            foc = cv::Vec3d(0,0,FOC_LEN);
            break;
        case QTools::ViewNavigator::LEFT:
            foc = cv::Vec3d(FOC_LEN,0,0);
            break;
        case QTools::ViewNavigator::REAR:
            foc = cv::Vec3d(0,0,-FOC_LEN);
            break;
        case QTools::ViewNavigator::RIGHT:
            foc = cv::Vec3d(-FOC_LEN,0,0);
            break;
        default:
            assert(false);
    }   // end switch
    return foc;
}   // end getFaceFocus


// public slot
void ViewFrame::showView( const View::Ptr v, ViewFace face)
{
    view_ = v;  // v can be NULL
    static const cv::Vec3d pos(0,0,0);
    static const cv::Vec3d up(0,1,0);
    cv::Vec3d foc = getFaceFocus( face);

    pcloudViewer_->resetPointCloud( pos, foc, up);
    imageViewer_->resetDepthFinder();

    if ( v != NULL)
    {
        setImage( v->img2d);
        imageViewer_->resetDepthFinder( v->points);
        imageViewer_->resetExtractHelper();
    }   // end if
}   // end showView
