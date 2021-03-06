/************************************************************************
 * Copyright (C) 2021 Richard Palmer
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

#ifndef QTOOLS_VTK_ACTOR_VIEWER_H
#define QTOOLS_VTK_ACTOR_VIEWER_H

#include "VtkViewerInteractorManager.h"
#include "QImageTools.h"
#include "KeyPressHandler.h"
#include <r3d/CameraParams.h>
#include <r3dvis/VtkTools.h>
#include <QApplication>     // All callers will have this anyway
#include <QVTKOpenGLNativeWidget.h>
#include <vtkInteractorStyle.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindow.h>
#include <iostream>
#include <vector>

namespace QTools {

using Vec3f = Eigen::Vector3f;

class QTools_EXPORT VtkActorViewer : public QVTKOpenGLNativeWidget
{ Q_OBJECT
public:
    explicit VtkActorViewer( QWidget *parent = nullptr);
    virtual ~VtkActorViewer();

    inline const vtkRenderWindow* getRenderWindow() const { return _rwin;}
    inline vtkRenderWindow* getRenderWindow() { return _rwin;}
    inline const vtkRenderer* getRenderer() const { return _ren;}
    inline vtkRenderer* getRenderer() { return _ren;}

    virtual void setInteractor( vtkInteractorStyle*);

    // Auto rendering of updates to the viewer is off by default.
    // It can be useful to set this on for simple view use cases (adding and removing single objects).
    // Otherwise, for more complicated scenarios, the client probably wants greater control over how
    // often the viewer re-renders the scene - and updateRender() should be called explicitly.
    void setAutoUpdateRender( bool autoRenderOn) { _autoUpdateRender = autoRenderOn;}
    bool isAutoUpdateRenderOn() const { return _autoUpdateRender;}
    virtual void updateRender();   // Render - CALL AFTER ALL CHANGES TO SEE UPDATES!

    // Reset the view window size.
    void setSize( size_t width, size_t height);
    inline size_t getWidth() const { return _ren->GetSize()[0];}
    inline size_t getHeight() const { return _ren->GetSize()[1];}
    inline cv::Size getSize() const { return cv::Size( (int)getWidth(), (int)getHeight());}

    cv::Mat_<float> getRawZBuffer();    // Grab snapshot of the raw Z buffer from the viewer.
    cv::Mat_<cv::Vec3b> getColourImg(); // Grab snapshot of whatever's currently displayed (3 byte BGR order)

    // Adding and removing props will cause a re-rendering of the scene if auto-rendering is on.
    void add( vtkProp*);
    void remove( vtkProp*);
    void clear();   // Remove all props

    // Will cause a re-rendering if auto rendering is on.
    void setStereoRendering( bool);	// Points are stereo if true else normal (automatic rendering update)
    void setOrthogonal( bool);      // Projection orthogonal if true else perspective (automatic rendering update)

    r3d::CameraParams camera() const;
    void setCamera( const r3d::CameraParams&);   // Move camera to given position

    // NOT called automatically!
    void refreshClippingPlanes();

    void setLights( const std::vector<r3dvis::Light>&);

    // Given a 2D render window (with TOP LEFT origin), returns true if the prop given is pointed at.
    bool pointedAt( const cv::Point&, const vtkProp*) const;
    bool pointedAt( const QPoint&, const vtkProp*) const;

    // Given a 2D render window (with TOP LEFT origin), find the actor from the given list being pointed to.
    // If no actors from the list are being pointed to, returns null.
    const vtkActor* pickActor( const cv::Point&, const std::vector<const vtkProp*>& pactors) const;
    const vtkActor* pickActor( const QPoint&, const std::vector<const vtkProp*>& pactors) const;

    // Given a 2D render window (with TOP LEFT origin), find the actor being pointed to
    // If no actor is found, return null.
    const vtkActor* pickActor( const cv::Point&) const;
    const vtkActor* pickActor( const QPoint&) const;

    // Find the position in 3D world space from a 2D point using TOP LEFT origin.
    // This returns the point that intersects the first pickable actor along the ray.
    // Note that this ALWAYS returns a valid position!
    Vec3f pickPosition( const cv::Point&) const;
    Vec3f pickPosition( const QPoint&) const;
    Vec3f pickPosition( const cv::Point2f&) const; // Point as proportion of window dims.

    bool pickPosition( const vtkProp*, const cv::Point&, Vec3f&) const;
    bool pickPosition( const vtkProp*, const QPoint&, Vec3f&) const;
    bool pickPosition( const vtkProp*, const cv::Point2f&, Vec3f&) const;  // Point as proportion of window dims.

    // Find the display position in 2D (using TOP LEFT origin) of a 3D world coordinate point.
    cv::Point projectToDisplay( const Vec3f&) const;
    QPoint projectToDisplayPoint( const Vec3f&) const;

    // As above, but return the coordinates from the top left of the display as a proportion of the display pane.
    cv::Point2f projectToDisplayProportion( const Vec3f&) const;


    /******************************************************************/
    /****************** MOUSE / KEYBOARD INTERACTION ******************/
    /******************************************************************/

    bool attach( VVI*); // Attach given interactor returning false iff already attached.
    bool detach( VVI*); // Detach given interactor returning false iff already detached.
    bool attach( VMH*); // Attach given mouse handler returning false iff already attached.
    bool detach( VMH*); // Detach given mouse handler returning false iff already detached.
    bool isAttached( VVI*) const;   // Returns whether the given interactor is attached.
    bool isAttached( VMH*) const;   // Returns whether the given mouse handler is attached.

    // Set/get the interaction mode (Camera or Actor)
    void setCameraInteraction() { _iman->setCameraInteraction();}
    void setActorInteraction( const vtkProp3D *p=nullptr) { _iman->setActorInteraction(p);}
    InteractionMode interactionMode() const { return _iman->interactionMode();}

    // Set/get locking of call pass-through for mouse events bound to camera/actor movement.
    // Interaction locking is key matched; interaction will not necessarily be unlocked if
    // more than one call has been made to lockInteraction(). All previous callers must call
    // unlockInteraction() with their held keys to unlock interaction. Function lockInteraction()
    // returns the key that should be passed to unlockInteraction() when that caller wishes
    // interaction to be unlocked. Function unlockInteraction() returns true iff unlocking worked.
    int lockInteraction() { return _iman->lockInteraction();}
    bool unlockInteraction( int key) { return _iman->unlockInteraction(key);}
    bool isInteractionLocked() const { return _iman->isInteractionLocked();}

    // Add/remove a key press handlers. Note that these functions are called as part of the
    // attach/detach process for VtkViewerInteractor instances if they define KeyPressHandlers.
    void attachKeyPressHandler( KeyPressHandler*);
    void detachKeyPressHandler( KeyPressHandler*);

protected:
    virtual void keyPressEvent( QKeyEvent*);
    virtual void keyReleaseEvent( QKeyEvent*);

private:
    bool _autoUpdateRender;
    VtkViewerInteractorManager *_iman;
    std::unordered_set<KeyPressHandler*> _keyPressHandlers;
    vtkNew<vtkRenderer> _ren;
    vtkNew<vtkGenericOpenGLRenderWindow> _rwin;
};	// end class

}	// end namespace

#endif
