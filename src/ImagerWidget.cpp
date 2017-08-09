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

#include <ImagerWidget.h>
using QTools::ImagerWidget;
#include "ui_ImagerWidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <sstream>
using std::ostringstream;
#include <iostream>
using std::cerr;
using std::endl;


ImagerWidget::ImagerWidget(QWidget *parent) : QWidget(parent),
    ui(new Ui::ImagerWidget)
{
    ui->setupUi(this);

    // Track mouse movements
    ui->imageLabel->setMouseTracking( true);
    ui->imageLabel->installEventFilter(this);

    // Save images
    connect( ui->saveImageButton, SIGNAL( clicked()), this, SLOT( saveImage()));

    extractHelper_ = RectangleManager::create();
    contextMenu_ = new QMenu( this);
    contextMenu_->addAction( tr("Extract image segment"), this, SLOT( extractImageSlot()));
    contextMenu_->addAction( tr("Extract feature"), this, SLOT( extractFeatureSlot()));
    contextMenu_->addAction( tr("Erase feature"), this, SLOT( eraseFeatureSlot()));
    contextMenu_->actions()[2]->setEnabled(false);

    // Create image context menu on right click
    ui->imageLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( ui->imageLabel, SIGNAL( customContextMenuRequested( const QPoint&)),
                       this, SLOT( createImageContextMenu( const QPoint&)));

    depthFinder_ = DepthFinder::Ptr();
}   // end ctor



ImagerWidget::~ImagerWidget()
{
    delete contextMenu_;
    delete ui;
}   // end dtor



cv::Mat ImagerWidget::getDisplayedImage() const
{
    const QPixmap *pixmap = ui->imageLabel->pixmap();
    return QTools::copyQImage2OpenCV( pixmap->toImage());
}   // end getDisplayedImage



// public
void ImagerWidget::enableFeatureExtract(bool enable)
{
    contextMenu_->actions()[1]->setEnabled(enable);
}   // end enableFeatureExtract



void ImagerWidget::setImage( const cv::Mat &img)
{
    curImg_ = img;
    //extractHelper_->reset();
    //clearRectangles();
    updateImage(curImg_);
}   // end setImage



void ImagerWidget::resetExtractHelper() const
{
    extractHelper_->reset();
}   // end resetExtractHelper



void ImagerWidget::refresh()
{
    updateImage(curImg_);
}   // end refresh



const cv::Mat ImagerWidget::getImage() const
{
    return curImg_;
}   // end getImage



void ImagerWidget::clearImage()
{
    const cv::Mat img = cv::Mat::zeros( 512, 512, CV_8UC1);
    setImage( img);
}   // end clearImage



void ImagerWidget::resetDepthFinder( const cv::Mat_<cv::Vec3f> points)
{
    depthFinder_ = DepthFinder::Ptr( new DepthFinder(points));
}   // end setDepthFinder



void ImagerWidget::resetDepthFinder()
{
    depthFinder_.reset();
}   // end resetDepthFinder



void ImagerWidget::setModelSize( const cv::Size2f &ms)
{
    modelSize_ = ms;
}   // setModelSize



// public
cv::Rect ImagerWidget::getSelectedRectangle() const
{
    return extractHelper_->getRectangle();
}   // end getExtractHelper



void ImagerWidget::shiftUserRectangle( int xDelta, int yDelta)
{
    extractHelper_->shiftHorizontally(xDelta);
    extractHelper_->shiftVertically(yDelta);
    drawRectangles();
}   // end shiftUserRectangle


void ImagerWidget::resizeUserRectangle( int horzDelta, int vertDelta)
{
    extractHelper_->resizeHorizontally( horzDelta);
    extractHelper_->resizeVertically( vertDelta);
    drawRectangles();
}   // end resizeUserRectangle


// public
void ImagerWidget::addRectangle( const cv::Rect &rct)
{
    otherRects_.push_back(rct);
    drawRectangles();
}   // end addRectangle


// public
void ImagerWidget::removeRectangle( const cv::Rect &rct)
{
    vector<cv::Rect> newRcts;
    foreach ( const cv::Rect& r, otherRects_)
    {
        if ( r != rct)
            newRcts.push_back(r);   // Copy to new array
    }   // end foreach
    // Recopy array to member array
    otherRects_ = newRcts;
    drawRectangles();
}   // end removeRectangle


// public
void ImagerWidget::clearRectangles()
{
    otherRects_.clear();
    drawRectangles();
}   // end clearRectangles



bool ImagerWidget::eventFilter( QObject *obj, QEvent *evt)
{
    if (evt->type() == QEvent::MouseMove)
        doImageMouseMoveEvent( dynamic_cast<QMouseEvent*>(evt));
    else if (evt->type() == QEvent::MouseButtonPress)
        doImageMousePressEvent( dynamic_cast<QMouseEvent*>(evt));
    else if (evt->type() == QEvent::MouseButtonRelease)
        doImageMouseReleaseEvent( dynamic_cast<QMouseEvent*>(evt));
    else if (evt->type() == QEvent::MouseButtonDblClick)
        doImageMouseDoubleClickEvent( dynamic_cast<QMouseEvent*>(evt));
    else if (evt->type() == QEvent::Leave)
    {
        showImageCoords( -1, -1);
        modelRect_ = cv::Rect(0,0,0,0);
    }   // end else if

    return false;   // Denote that event not swallowed and forwarded up to the imageLabel
    //return ui->imageLabel->eventFilter( obj, evt);  // Forward up to the imageLabel
}   // end eventFilter



void ImagerWidget::saveImage()
{
    const cv::Mat im = getDisplayedImage();
    QTools::saveImage( im);
}   // end saveImage



// private slot
void ImagerWidget::createImageContextMenu( const QPoint &pos)
{
    // Only display if point is within a rectangle
    const cv::Point pt(pos.x(), pos.y());
    bool inRect = false;
    foreach ( const cv::Rect& r, otherRects_)
    {
        if ( r.contains(pt))
        {
            inRect = true;
            break;
        }   // end if
    }   // end foreach

    if (!inRect)    // Check if point within currently drawing rectangle
        inRect = extractHelper_->intersects(pos.x(), pos.y());

    if ( inRect)
        contextMenu_->exec( ui->imageLabel->mapToGlobal(pos));
}   // end createImageContextMenu



void ImagerWidget::doImageMousePressEvent( QMouseEvent* evt)
{
    contextMenu_->actions()[2]->setEnabled(false);
    const int x = evt->pos().x();
    const int y = evt->pos().y();

    if ( evt->button() == Qt::LeftButton)
    {
        // Depending on where user clicked, they might want to resize the rectangle
        // or move it to a different position. If the user clicked outside of the
        // rectangle, the rectangle is reset.
        if ( extractHelper_->onCorner( x, y, 4))
            extractHelper_->resize( x, y);
        else if ( extractHelper_->onHorizontalEdge( x, y, 4))
            extractHelper_->resizeVert( x, y);
        else if ( extractHelper_->onVerticalEdge( x, y, 4))
            extractHelper_->resizeHorz( x, y);
        else if ( extractHelper_->intersects( x, y))
            extractHelper_->move( x, y);
        else if ( !extractHelper_->working())
        {   // Create a new rectangle
            extractHelper_->createNew( x, y);
            updateCursorIcon( x, y);
        }   // end if
        else
            extractHelper_->reset();

        updateCursorIcon( x, y);
    }   // end if
    else
    {
        // Find out which (if any) feature rectangles contain this point.
        // If so, allow for rectangle erasure.
        foreach ( const cv::Rect& rct, otherRects_)
        {
            const cv::Point pt(x,y);
            if ( rct.contains(pt))
            {
                contextMenu_->actions()[2]->setEnabled(true);
                eraseRect_ = rct;
            }   // end if
        }   // end foreach
    }   // end else
}   // end doImageMousePressEvent



void ImagerWidget::doImageMouseReleaseEvent( QMouseEvent* evt)
{
    if ( evt->button() == Qt::LeftButton)
    {
        extractHelper_->stop();
        updateCursorIcon( evt->pos().x(), evt->pos().y());
    }   // end if
}   // end doImageMouseReleaseEvent



void ImagerWidget::doImageMouseDoubleClickEvent( QMouseEvent* evt)
{
    // Double click causes a new rectangle to start being drawn at the cursor's position:
    if ( evt->button() == Qt::LeftButton)
    {
    ;   // No-op
    }   // end if
}   // end doImageMouseDoubleClickEvent



void ImagerWidget::doImageMouseMoveEvent( QMouseEvent* evt)
{
    const QPoint &pos = evt->pos();
    const int x = pos.x();
    const int y = pos.y();
    showImageCoords( x, y);
    if ( depthFinder_ != NULL && modelSize_.height > 0 && modelSize_.width > 0)
        depthFinder_->calcModelRect(modelSize_, x, y, modelRect_, 20);

    extractHelper_->update( x, y);
    updateCursorIcon(x,y);
}   // end doImageMouseMoveEvent



void ImagerWidget::showImageCoords( int x, int y)
{
    if ((x == -1) || (y == -1))
        ui->coordsLabel->setText("");
    else
    {
        ostringstream oss;
        oss << x << ", " << y;
        ui->coordsLabel->setText( tr(oss.str().c_str()));
    }	// end else
}	// end showImageCoords



void ImagerWidget::drawRectangles()
{
    cv::Mat dimg = curImg_.clone();

    // Place the drawn rectangle on the image if set
    cv::Rect rct = extractHelper_->getRectangle();
    if ( rct.area() > 0)
        cv::rectangle( dimg, rct, cv::Scalar(90,255,110), 1);

    emit showingUserRectangle( rct);

    // Place the model rectangle on the display image if set
    // and the user is not currently manipulating the extract region
    if ( !extractHelper_->working() && modelRect_.area() > 0)
        cv::rectangle( dimg, modelRect_, cv::Scalar(0,50,255), 3);

    // Overlay all other added rectangles
    foreach ( const cv::Rect &r, otherRects_)
        cv::rectangle( dimg, r, cv::Scalar(255,0,0), 2);

    updateImage( dimg);
}   // end drawRectangles



void ImagerWidget::updateImage( const cv::Mat &dimg)
{
    // Set the new image into the image label
    QImage qimg = QTools::copyOpenCV2QImage( dimg);
    QPixmap pixmap = QPixmap::fromImage( qimg);
    ui->imageLabel->setPixmap( pixmap);
}   // end updateImage



void ImagerWidget::updateCursorIcon( int x, int y)
{
    if ( extractHelper_->isResizing() || extractHelper_->isDrawing())
        ui->imageLabel->setCursor(Qt::CrossCursor);
    else
    {
        const bool leftEdge = extractHelper_->onLeftEdge( x, y, 4);
        const bool rightEdge = extractHelper_->onRightEdge( x, y, 4);
        const bool topEdge = extractHelper_->onTopEdge( x, y, 4);
        const bool bottomEdge = extractHelper_->onBottomEdge( x, y, 4);

        if ( leftEdge && topEdge || rightEdge && bottomEdge)
            ui->imageLabel->setCursor(Qt::SizeFDiagCursor);
        else if ( leftEdge && bottomEdge || rightEdge && topEdge)
            ui->imageLabel->setCursor(Qt::SizeBDiagCursor);
        else if ( topEdge || bottomEdge)
            ui->imageLabel->setCursor(Qt::SizeVerCursor);
        else if ( leftEdge || rightEdge)
            ui->imageLabel->setCursor(Qt::SizeHorCursor);
        else if ( extractHelper_->intersects( x, y))
        {
            if ( extractHelper_->isMoving())
                ui->imageLabel->setCursor(Qt::ClosedHandCursor);
            else
                ui->imageLabel->setCursor(Qt::OpenHandCursor);
        }   // end if
        else
            ui->imageLabel->setCursor(Qt::ArrowCursor);
    }   // end else

    drawRectangles();
}   // end updateCursorIcon



// private slot
void ImagerWidget::extractImageSlot()
{
    emit extractImage( extractHelper_->getRectangle());
}   // extractImageSlot


// private slot
void ImagerWidget::extractFeatureSlot()
{
    emit extractFeature( extractHelper_->getRectangle());
}   // end extractFeatureSlot



// private slot
void ImagerWidget::eraseFeatureSlot()
{
    emit eraseFeature( eraseRect_);
}   // end eraseFeatureSlot
