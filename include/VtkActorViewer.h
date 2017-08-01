#ifndef QTOOLS_VTK_ACTOR_VIEWER_H
#define QTOOLS_VTK_ACTOR_VIEWER_H

#include <iostream>
#include <vector>
#include "QImageTools.h"
#include "QTools_Export.h"
#include <CameraParams.h>   // RFeatures
#include <QVTKWidget.h>
#include <vtkSmartPointer.h>
#include <vtkInteractorStyle.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

namespace RVTK { class RendererPicker;}


namespace QTools
{

class QTools_EXPORT VtkActorViewer : public QVTKWidget
{ Q_OBJECT
public:
    VtkActorViewer( QWidget *parent = NULL, bool offscreenRendering=false);
    virtual ~VtkActorViewer();

    vtkRenderer* getRenderer() { return _ren;}
    virtual void setInteractor( vtkSmartPointer<vtkInteractorStyle>);

    // Auto rendering of updates to the viewer is off by default.
    // It can be useful to set this on for simple view use cases (adding and removing single objects).
    // Otherwise, for more complicated scenarios, the client probably wants greater control over how
    // often the viewer re-renders the scene - and updateRender() should be called explicitly.
    void setAutoUpdateRender( bool autoRenderOn) { _autoUpdateRender = autoRenderOn;}
    bool isAutoUpdateRenderOn() const { return _autoUpdateRender;}
    virtual void updateRender();   // Render - CALL AFTER ALL CHANGES TO SEE UPDATES!

    // Reset the view window size.
    void setSize( int width, int height);
    int getWidth() const;
    int getHeight() const;
    cv::Size getSize() const;

    cv::Mat_<float> getRawZBuffer() const; // Grab snapshot of the raw Z buffer from the viewer.
    cv::Mat_<cv::Vec3b> getColourImg() const; // Grab snapshot of whatever's currently displayed (3 byte BGR order)

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
    // See CameraParams default ctor for default initial reset camera params.
    void setResetCamera( const RFeatures::CameraParams&);
    void resetCamera(); // Reset camera parameters to those set in last call to setResetCamera

    void getCamera( RFeatures::CameraParams&) const;
    void setCamera( const RFeatures::CameraParams&);   // Move camera to given position

    // Set the lighting array for the scene - each light must have a corresponding focal point.
    void setSceneLights( const std::vector<cv::Vec3f>& lightPositions, const std::vector<cv::Vec3f>& lightFocalPoints);
    void setHeadlight();    // Replace scene lights (if present) with a camera headlight

    // Given a 2D render window (with TOP LEFT origin), find the actor from the given list being pointed to.
    // If no actors from the list are being pointed to, returns NULL.
    vtkActor* pickActor( const cv::Point& point, const std::vector<vtkActor*>& pactors) const;
    vtkSmartPointer<vtkActor> pickActor( const cv::Point& point, const std::vector<vtkSmartPointer<vtkActor> >& pactors) const;

    // Given a 2D render window (with TOP LEFT origin), find the actor being pointed to
    // If no actor is found, return NULL.
    vtkActor* pickActor( const cv::Point& point) const;

    // Find an actor's cell addressed by a 2D point (using TOP LEFT origin).
    // If no actor cell is found (no actor is pointed to), -1 is returned.
    int pickCell( const cv::Point& point) const;

    // Given a vector of 2D points (using TOP LEFT origin) and an actor (cannot be NULL), set cellIds with
    // the indices of the cells intercepted by the points. Duplicate cellIds are ignored. Returns the number
    // of cell IDs appended to cellIds.
    int pickActorCells( const std::vector<cv::Point>& points, vtkActor* actor, std::vector<int>& cellIds) const;

    // As above but selects cells where the corresponding 2D mask values > 0.
    int pickActorCells( const cv::Mat& mask, vtkActor* actor, std::vector<int>& cellIds) const;

    // Find the position in 3D world space from a 2D point using TOP LEFT origin.
    cv::Vec3f pickWorldPosition( const cv::Point& p) const;

    // As above, but specify view coordinates from top left as a proportion of the window dimensions.
    cv::Vec3f pickWorldPosition( const cv::Point2f& p) const;

    // Pick the surface normal at the given 2D point having TOP LEFT origin.
    // If no surface is picked at p, (0,0,0) is returned.
    cv::Vec3f pickNormal( const cv::Point& p) const;

    // Find the display position in 2D (using TOP LEFT origin) of a 3D world coordinate point.
    cv::Point projectToDisplay( const cv::Vec3f& v) const;

    // As above, but return the coordinates from the top left of the display as a proportion of the display pane.
    cv::Point2f projectToDisplayProportion( const cv::Vec3f& v) const;

private:
    bool _autoUpdateRender;
    mutable vtkRenderer* _ren;
    mutable vtkRenderWindow* _rwindow;
    RVTK::RendererPicker *_rpicker;
    RFeatures::CameraParams _resetCamera;
};	// end class

}	// end namespace

#endif
