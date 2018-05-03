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
 * events to non VTK based delegate objects derived from ModelViewerInteractor for implementing
 * different interactions. VTK interaction modes can be switched between trackball camera (the default)
 * and trackball actor. NB Unlike VTK interaction, the right mouse button instead of the middle mouse
 * button is used for panning.
 *
 * Descendents of ModelViewerInteractor can enable/disable interactions by overriding the requisite
 * function stubs in ModelViewerInteractor. More than one ModelViewerInteractor can be attached and
 * the attaching client must manage the priority of the attached ModelViewerInteractor objects.
 */

#include "VtkViewerInteractor.h"
#include "VtkActorViewer.h"
#include <QPoint>

namespace QTools {

enum InteractionMode
{
    CAMERA_INTERACTION,
    ACTOR_INTERACTION
};  // end enum

class QTools_EXPORT VtkViewerInteractorManager
{
public:
    explicit VtkViewerInteractorManager( VtkActorViewer*);
    virtual ~VtkViewerInteractorManager(){}

    const std::unordered_set<VVI*>& interactors() const;
    void addInteractor( VVI*);
    void removeInteractor( VVI*);

    QPoint getMouseCoords();    // With top left origin

    // Lock/unlock camera/actor interaction.
    void setInteractionLocked( bool);
    bool isInteractionLocked() const;

    // Camera mode can be camera transform (default), or actor tansform.
    // Interaction simply moves the underlying actors on the GPU and
    // adjusts their internal transformations. Poly data for actors must
    // be updated externally.
    void setInteractionMode( InteractionMode);
    InteractionMode interactionMode() const { return _imode;}

    void doOnLeftButtonDown();
    void doOnLeftButtonUp();
    void doOnMiddleButtonDown();
    void doOnMiddleButtonUp();
    void doOnRightButtonDown();
    void doOnRightButtonUp();
    void doOnMouseWheelForward();
    void doOnMouseWheelBackward();
    
    void doOnMouseMove();
    void doOnEnter();
    void doOnLeave();

private:
    VtkActorViewer *_qviewer;
    bool _locked, _lbdown, _rbdown, _mbdown;
    InteractionMode _imode;
    qint64 _lbDownTime;
    std::unordered_set<VVI*> _ifaces;   // The interfaces
    std::unordered_map<int, vtkSmartPointer<vtkInteractorStyle> > _interactors;

    VtkViewerInteractorManager( const VtkViewerInteractorManager&); // No copy
    void operator=( const VtkViewerInteractorManager&);             // No copy
};  // end class

}   // end namespace

#endif
