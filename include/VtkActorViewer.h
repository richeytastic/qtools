#pragma once
#ifndef QTOOLS_VTK_ACTOR_VIEWER_H
#define QTOOLS_VTK_ACTOR_VIEWER_H

#include <Viewer.h>         // RVTK
#include <vector>
using std::vector;
#include "QImageTools.h"
#include "QTools_Export.h"

#include <QVTKWidget.h>


namespace QTools
{

class QTools_EXPORT VtkActorViewer : public QVTKWidget
{ Q_OBJECT
public:
    explicit VtkActorViewer( QWidget *parent = NULL);
    ~VtkActorViewer();

    void setInteractor( vtkSmartPointer<vtkInteractorStyle>);

    // Auto rendering of updates to the viewer is off by default.
    // It can be useful to set this on for simple view use cases (adding and removing single objects).
    // Otherwise, for more complicated scenarios, the client probably wants greater control over how
    // often the viewer re-renders the scene - and updateRender() should be called explicitly.
    inline void setAutoUpdateRender( bool autoRenderOn) { _autoUpdateRender = autoRenderOn;}
    inline bool isAutoUpdateRenderOn() const { return _autoUpdateRender;}
    virtual void updateRender() { _viewer->updateRender();}   // Render - CALL AFTER ALL CHANGES TO SEE UPDATES!

    // Reset the view window size.
    void setSize( int width, int height);
    int getWidth() const { return _viewer->getWidth();}
    int getHeight() const { return _viewer->getHeight();}

    // Adding and removing actors will cause a re-rendering of the scene if auto-rendering is on.
    void addActor( vtkActor*);
    void removeActor( vtkActor*);
    void clear();   // Remove all object actors

    // Will cause a re-rendering if auto rendering is on.
    void setBackgroundWhite( bool v);	// Background white if true, else black (automatic rendering update)
    void setStereoRendering( bool v);	// Points are stereo if true else normal (automatic rendering update)
    void setOrthogonal( bool v);        // Projection orthogonal if true else perspective (automatic rendering update)

    // All camera adjustments will cause a re-rendering if auto-rendering is on.
    // Sets the parameters to reset the camera to when calling reset camera.
    // Default values are all zero'd out which probably isn't what you want.
    void setResetCamera( const cv::Vec3f &pos, const cv::Vec3f &focus, const cv::Vec3f &up);
    void resetCamera(); // Reset camera parameters
    // Move the camera parameters to a new position
    void adjustCamera( const cv::Vec3f &pos, const cv::Vec3f &focus, const cv::Vec3f &up);
    void printCameraParams() const;

    void setCameraFocus( const cv::Vec3f& focus); // Set the focus only
    void setCameraPosition( const cv::Vec3f& pos);  // Set the position only
    void setCameraViewUp( const cv::Vec3f& up); // Set the up vector only

    cv::Vec3f getCameraFocus() const { return _viewer->getCameraFocus();}
    cv::Vec3f getCameraPosition() const { return _viewer->getCameraPosition();}
    cv::Vec3f getCameraViewUp() const { return _viewer->getCameraViewUp();}
    double getCameraFieldOfView() const { return _viewer->getFieldOfView();}

    // Set the lighting array for the scene - each light must have a corresponding focal point.
    void setLighting( const vector<cv::Vec3f>& lightPositions, const vector<cv::Vec3f>& lightFocalPoints, bool headlightEnabled=false);
    void setHeadlight( bool enabled);

    // Grab a snapshot of whatever's currently displayed (3 byte BGR order)
    cv::Mat_<cv::Vec3b> grabImage() const;

    // Given a 2D render window (with TOP LEFT origin), find the actor from the given list being pointed to.
    // If no actors from the list are being pointed to, returns NULL.
    vtkSmartPointer<vtkActor> pickActor( const cv::Point& point, const vector<vtkSmartPointer<vtkActor> >& pactors) const;

    // Given a 2D render window (with TOP LEFT origin), find the actor being pointed to
    // If no actor is found, return NULL.
    vtkSmartPointer<vtkActor> pickActor( const cv::Point& point) const;

    // Find an actor's cell addressed by a 2D point (using TOP LEFT origin).
    // If no actor cell is found (no actor is pointed to), -1 is returned.
    int pickCell( const cv::Point& point) const;

    // Given a vector of 2D points (using TOP LEFT origin) and an actor (cannot be NULL), set cellIds with
    // the indices of the cells intercepted by the points. Duplicate cellIds are ignored. Returns the number
    // of cell IDs appended to cellIds.
    int pickActorCells( const vector<cv::Point>& points, const vtkActor* actor, vector<int>& cellIds) const;

    // As above but selects cells where the corresponding 2D mask values > 0.
    int pickActorCells( const cv::Mat& mask, const vtkActor* actor, vector<int>& cellIds) const;

    // Find the position in 3D world space from a 2D point using TOP LEFT origin.
    cv::Vec3f pickWorldPosition( const cv::Point& p) const;

    // As above, but specify view coordinates from top left as a proportion of the window dimensions.
    cv::Vec3f pickWorldPosition( const cv::Point2f& p) const;

    // As above, but look at a pixel neighbourhood that's a 4-connected cross including 4.
    // Nowhere near as fast, but better if possible holes to contend with.
    cv::Vec3f pickWorldMeanPosition( const cv::Point& p) const;

    // Pick the surface normal at the given 2D point having TOP LEFT origin.
    // If no surface is picked at p, (0,0,0) is returned.
    cv::Vec3f pickNormal( const cv::Point& p) const;

    // Find the display position in 2D (using TOP LEFT origin) of a 3D world coordinate point.
    cv::Point projectToDisplay( const cv::Vec3f& v) const;

    // As above, but return the coordinates from the top left of the display as a proportion of the display pane.
    cv::Point2f projectToDisplayProportion( const cv::Vec3f& v) const;

protected:
    RVTK::Viewer::Ptr getViewer() const { return _viewer;}

private:
    RVTK::Viewer::Ptr _viewer;
    bool _autoUpdateRender;

    // Reset camera params
    cv::Vec3f _camPos;
    cv::Vec3f _camFocus;
    cv::Vec3f _camUp;
};	// end class

}	// end namespace

#endif
