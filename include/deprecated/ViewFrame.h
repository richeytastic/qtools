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

#ifndef QTOOLS_VIEW_FRAME_H__
#define QTOOLS_VIEW_FRAME_H__

#include <QFrame>
#include "QTools_Export.h"
#include "PointCloudViewer.h"
#include "ImagerWidget.h"
#include "ViewNavigator.h"
typedef QTools::ViewNavigator::ViewFace ViewFace;

#include <View.h>
using RFeatures::View;


namespace Ui {
class ViewFrame;
}

namespace QTools
{

class QTools_EXPORT ViewFrame : public QFrame
{ Q_OBJECT
public:
    explicit ViewFrame(QWidget *parent = 0);
    ~ViewFrame();

    // Returns the rectangle selected by the user (if available)
    // otherwise a rectangle with area=0 is returned.
    cv::Rect getSelectedRectangle() const;

signals:
    void showingUserRectangle( cv::Rect);
    void extractImage( cv::Rect);
    void extractFeature( cv::Rect);
    void eraseFeature( cv::Rect);

public slots:
    void showFeatureRectangle( const cv::Rect&);
    void clearFeatureRectangle( const cv::Rect&);
    void clearFeatureRectangles();
    void enableFeatureExtract( bool);

    void shiftUserRectangle( int xDelta, int yDelta);
    void resizeUserRectangle( int horzDelta, int vertDelta);

    void viewImage();   // Set view to the 2D image
    void viewDepth();   // Set view to the depth image
    void viewPoints();  // Set view to the 3D point cloud
    void reset();       // Reset to no view

    void setImage( const cv::Mat&); // Set an arbitrary 2D image to display
    cv::Mat getImage() const;
    void setModelSize( const cv::Size2f&);  // Set a model

    // Add points to the viewer (with proportion of total so far)
    void addPoints( const cv::Mat_<cv::Vec3f> pointCloud, const cv::Mat_<cv::Vec3b> colourImg, double prop);

    void showView( const View::Ptr, ViewFace);

private:
    Ui::ViewFrame *ui;

    int totPoints_;
    QTools::PointCloudViewer *pcloudViewer_;
    QTools::ImagerWidget *imageViewer_;
    View::Ptr view_;    // Current view being displayed

    cv::Vec3d getFaceFocus( ViewFace);
};  // end class

}   // end namespace

#endif
