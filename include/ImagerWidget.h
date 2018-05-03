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

#ifndef QTOOLS_IMAGER_WIDGET_H
#define QTOOLS_IMAGER_WIDGET_H

#include <vector>
using std::vector;

#include <QWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPoint>

#include "QTools_Export.h"
#include "QImageTools.h"

#include <RectangleManager.h>
using RFeatures::RectangleManager;
#include <DepthFinder.h>
using RFeatures::DepthFinder;

#include <opencv2/opencv.hpp>
#include <boost/shared_ptr.hpp>


namespace Ui {
    class ImagerWidget;
}

namespace QTools {

class QTools_EXPORT ImagerWidget : public QWidget
{ Q_OBJECT
public:
    explicit ImagerWidget(QWidget *parent = 0);
    ~ImagerWidget();

    // Returns a copy of the currently displayed image as seen by the user
    // (which may be different to the set image due to extra information
    // being drawn to the set image prior to rendering).
    cv::Mat getDisplayedImage() const;

    // Enable/disable option to extract features
    void enableFeatureExtract( bool);

    void setImage( const cv::Mat&);

    void resetExtractHelper() const;

    void refresh(); // Refresh using the currently set version of the image

    // Returns the image set in the widget. Note that this function
    // returns a reference to the internal image matrix rather than
    // a copy (so don't adjust the returned copy!).
    const cv::Mat getImage() const;

    void clearImage();  // Set the displayed image to black

    void resetDepthFinder( const cv::Mat_<cv::Vec3f> points);
    void resetDepthFinder();
    void setModelSize( const cv::Size2f&);

    // Returns the rectangle currently selected by internal rectangle manager
    // or a rectangle with zero area if no rectangle selected.
    cv::Rect getSelectedRectangle() const;

    void shiftUserRectangle( int xDelta, int yDelta);
    void resizeUserRectangle( int horzDelta, int vertDelta);

    // Overlay the given rectangle on the image. All such rectangles are reset either
    // on a call to setImage or by calling clearRectangles below.
    void addRectangle( const cv::Rect&);
    void removeRectangle( const cv::Rect&);
    void clearRectangles();

signals:
    void showingUserRectangle( cv::Rect); // Emitted if just drawn a user defined rectangle on the image
    void extractImage( cv::Rect);   // Emitted if want to extract a segment from the image
    void extractFeature( cv::Rect); // Emitted if want to extract a feature from the view
    void eraseFeature( cv::Rect);   // Emitted if want to erase a displayed feature from the view

protected:
    virtual bool eventFilter( QObject *obj, QEvent *evt); // Track mouse events to draw rectangles on the image

private slots:
    void saveImage();
    void createImageContextMenu( const QPoint&);
    void extractFeatureSlot();
    void extractImageSlot();
    void eraseFeatureSlot();

private:
    Ui::ImagerWidget *ui;
    QMenu *contextMenu_;
    RectangleManager::Ptr extractHelper_; // Tracks rectangles drawn by user to save extracts

    cv::Mat curImg_;     // Current image to display
    DepthFinder::Ptr depthFinder_;
    // Rectangle denoting position of model in image (only if an object model loaded)
    cv::Rect modelRect_;
    cv::Size2f modelSize_;   // Model actual size (in metres)

    vector<cv::Rect> otherRects_;   // Other (arbitrary) rectangles
    cv::Rect eraseRect_;    // Rectangle selected for possible erasure

    // For tracking the drawing of rectangles and setting of modelRect
    void doImageMousePressEvent( QMouseEvent* event);
    void doImageMouseReleaseEvent( QMouseEvent* event);
    void doImageMouseDoubleClickEvent( QMouseEvent* event);
    void doImageMouseMoveEvent( QMouseEvent* event);

    void showImageCoords( int x, int y);
    void drawRectangles();  // Extract and model rectangles
    void updateImage( const cv::Mat&);
    void updateCursorIcon( int x, int y);
};  // end class

}   // end namespace

#endif
