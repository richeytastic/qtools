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

#ifndef QTOOLS_VTK_VIEWER_INTERACTOR_MANAGER_H
#define QTOOLS_VTK_VIEWER_INTERACTOR_MANAGER_H

/**
 * Not actually a vtkInteractor, but manages the application of different vtkInteractorStyles to a
 * vtkRenderWindow (via its implementation as VtkActorViewer). This class communicates VTK mouse
 * events to non VTK based delegate objects derived from VtkViewerInteractor for implementing
 * different interactions. VTK interaction modes can be switched between trackball camera (the default)
 * and trackball actor. Unlike VTK interaction, the right mouse button instead of the middle mouse
 * button is used for panning.
 *
 * Descendents of VtkViewerInteractor can enable/disable interactions by overriding the requisite
 * function stubs in that class. More than one VtkViewerInteractor can be attached and the attaching
 * client must manage the priority of the attached VtkViewerInteractor objects.
 */

#include "VtkMouseHandler.h"
#include "VtkViewerInteractor.h"
#include "VtkViewerSwitchInteractor.h"
#include <QPoint>
#include <vtkSmartPointer.h>
#include <unordered_set>
#include <unordered_map>
#include <random>

namespace QTools {

class VtkActorViewer;

enum InteractionMode
{
    CAMERA_INTERACTION,
    ACTOR_INTERACTION
};  // end enum

class QTools_EXPORT VtkViewerInteractorManager
{
public:
    explicit VtkViewerInteractorManager( VtkActorViewer*);

    void addInteractor( VVI*);
    void removeInteractor( VVI*);
    bool isAttached( VVI* v) const { return _vvis.count(v) > 0;}

    void addMouseHandler( VMH*);
    void removeMouseHandler( VMH*);
    bool isAttached( VMH* v) const { return _vmhs.count(v) > 0;}

    // Lock/unlock camera/actor interaction. Locking key matched since many interactors
    // may be active; one interactor should not be able to unlock interaction when another
    // interactor requires interaction to still be locked.
    int lockInteraction();  // Returns the key to unlock
    // Pass in key returned from lockInteraction. Returns true if unlocking worked (or not yet locked).
    bool unlockInteraction( int key);
    // Returns true iff interaction currently locked.
    bool isInteractionLocked() const;

    // Camera mode can be camera transform (default), or actor tansform. Interaction moves the
    // underlying actors on the GPU and adjusts their internal transformations. Poly data for actors
    // must be updated externally.
    // If in actor interaction mode, no movement will occur if no actor is picked under the mouse cursor.
    // Use useCamOffActor=true to revert to camera interaction if in actor interaction mode and no
    // actor is under the cursor. Otherwise, no movement will occur until set back in camera mode.
    void setInteractionMode( InteractionMode, bool useCameraOffActor=false);
    InteractionMode interactionMode() const { return _imode;}
    bool useCameraOffActor() const { return _iswitch->useCameraOffActor();}

private:
    bool doOnLeftButtonDown();
    bool doOnLeftButtonUp();
    bool doOnMiddleButtonDown();
    bool doOnMiddleButtonUp();
    bool doOnRightButtonDown();
    bool doOnRightButtonUp();
    bool doOnMouseWheelForward();
    bool doOnMouseWheelBackward();
    bool doOnMouseMove();

    void doOnEnter();
    void doOnLeave();

    void doBeforeCameraStart();
    void doAfterCameraRotate();
    void doAfterCameraPan();
    void doAfterCameraDolly();
    void doAfterCameraSpin();
    void doAfterCameraStop();

    void doBeforeActorStart( const vtkProp3D*);
    void doAfterActorRotate( const vtkProp3D*);
    void doAfterActorDolly( const vtkProp3D*);
    void doAfterActorSpin( const vtkProp3D*);
    void doAfterActorPan( const vtkProp3D*);
    void doAfterActorStop( const vtkProp3D*);

    friend class VtkViewerSwitchInteractor;

    VtkActorViewer *_qviewer;
    bool _lbdown, _rbdown, _mbdown;
    InteractionMode _imode;
    qint64 _lbDownTime;
    std::minstd_rand0 _rng; // Random number generator for _lockKeys

    vtkNew<VtkViewerSwitchInteractor> _iswitch;
    std::unordered_set<VVI*> _vvis;   // The interactors
    std::unordered_set<VMH*> _vmhs;   // The mouse handlers
    std::unordered_set<int> _lockKeys;

    VtkViewerInteractorManager( const VtkViewerInteractorManager&) = delete;
    void operator=( const VtkViewerInteractorManager&) = delete;
};  // end class

}   // end namespace

#endif
