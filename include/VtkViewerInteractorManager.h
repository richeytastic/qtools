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
    virtual ~VtkViewerInteractorManager();

    const std::unordered_set<VVI*>& interactors() const;
    void addInteractor( VVI*);
    void removeInteractor( VVI*);

    QPoint getMouseCoords();    // With top left origin

    // Lock/unlock camera/actor interaction. Locking key matched since many interactors
    // may be active; one interactor should not be able to unlock interaction when another
    // interactor requires interaction to still be locked.
    int lockInteraction();  // Returns the key to unlock
    // Pass in key returned from lockInteraction. Returns true if unlocking worked (or not yet locked).
    bool unlockInteraction( int key);
    // Returns true iff interaction currently locked.
    bool isInteractionLocked() const;

    // Camera mode can be camera transform (default), or actor tansform.
    // Interaction simply moves the underlying actors on the GPU and
    // adjusts their internal transformations. Poly data for actors must
    // be updated externally.
    void setInteractionMode( InteractionMode);
    InteractionMode interactionMode() const { return _imode;}

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
    bool doOnEnter();
    bool doOnLeave();

    void doAfterCameraRotate();
    void doAfterCameraPan();
    void doAfterCameraDolly();
    void doAfterCameraSpin();
    void doAfterCameraStop();

    void doAfterActorRotate();
    void doAfterActorDolly();
    void doAfterActorSpin();
    void doAfterActorPan();
    void doAfterActorStop();

    friend class VtkViewerSwitchInteractor;

    VtkActorViewer *_qviewer;
    bool _lbdown, _rbdown, _mbdown;
    InteractionMode _imode;
    qint64 _lbDownTime;
    std::minstd_rand0 _rng; // Random number generator for _lockKeys

    VtkViewerSwitchInteractor* _istyle;
    std::unordered_set<VVI*> _vvis;   // The interactors
    std::unordered_set<int> _lockKeys;

    VtkViewerInteractorManager( const VtkViewerInteractorManager&); // No copy
    void operator=( const VtkViewerInteractorManager&);             // No copy
};  // end class

}   // end namespace

#endif
