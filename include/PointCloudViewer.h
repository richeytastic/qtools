#pragma once
#ifndef QTOOLS_POINT_CLOUD_VIEWER_H
#define QTOOLS_POINT_CLOUD_VIEWER_H

#include <Viewer.h>
#include <VtkDisplayActor.h>
#include <PointCloudMapper.h>
#include <LinesMapper.h>
using RVTK::VtkDisplayActor;
using RVTK::LinesMapper;
using RVTK::PointCloudMapper;
using RVTK::Viewer;

#include "QTools_Export.h"
#include "QImageTools.h"
using QTools::QImageTools;

#include <KeypointsConverter.h>

#include <opencv2/opencv.hpp>
#include <QWidget>
#include <vtkEventQtSlotConnect.h>


namespace Ui {
    class PointCloudViewer;
}


namespace QTools
{

class QTools_EXPORT PointCloudViewer : public QWidget
{ Q_OBJECT
public:
    explicit PointCloudViewer( QWidget *parent = NULL);
    ~PointCloudViewer();

    void setViewArea( int width, int height);

    // Get the currently displayed view as an image (3 byte BGR order)
    cv::Mat_<cv::Vec3b> getImage() const;

public slots:
    void setBackgroundWhite( bool v);	// Background white if true, else black (automatic rendering update)
    void setStereoRendering( bool v);	// Points are stereo if true else normal (automatic rendering update)
    void setOrthogonal( bool v);		// Projection orthogonal if true else perspective (automatic rendering update)
    void resetCamera();               // Reset camera parameters (position, focal point, view up vector)

    void resetPointCloud( const cv::Vec3d &pos, const cv::Vec3d &focus, const cv::Vec3d &up);
    void addPoints( const cv::Mat_<cv::Vec3f>, const cv::Mat_<cv::Vec3b>);
    void setProgressBar( uint pcnt, uint count);

    // Set point cloud of keypoints (updates rendering)
    void setLines( const Lines &lines, const cv::Mat &rngData, int minSegLen, double minFitCoeff);
    void setCircles( Circles &circles);

    void clear();	// Reset to a blank canvas (does NOT update rendering!)
    void updateRender();    // Force rendering update

private slots:
    void updateCoords( vtkObject *obj);
    void displayContextMenu( vtkObject *obj, unsigned long, void *clientData, void*, vtkCommand *command);
    void saveImage() const;

private:
    Ui::PointCloudViewer* ui;
    QAction* toggleBg;	// Background action toggle
    QAction* toggleSt;	// Stereo action toggle
    QAction* togglePr;	// Projection action toggle
    vtkEventQtSlotConnect* connections;

    Viewer::Ptr vtkViewer;

    VtkDisplayActor::Ptr pointCloudActor;
    VtkDisplayActor::Ptr linesActor;

    PointCloudMapper::Ptr pointCloudMapper;
    LinesMapper::Ptr linesMapper;

    // Initial camera parameters on instantiation of a new point cloud actor
    cv::Vec3d camPos;
    cv::Vec3d camFocus;
    cv::Vec3d camUp;

    // Convenience functions for constructor
    void createContextMenu();	// Including actions
    void connectMouseEvents();

    void saveCameraParameters( const cv::Vec3d &pos, const cv::Vec3d &focus, const cv::Vec3d &up);
    void setOpaque( VtkDisplayActor::Ptr actor);
    void setTransparent( VtkDisplayActor::Ptr actor);
};	// end class

}	// end namespace

#endif
