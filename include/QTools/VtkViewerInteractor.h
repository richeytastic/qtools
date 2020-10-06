/************************************************************************
 * Copyright (C) 2019 Richard Palmer
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

#ifndef QTOOLS_VTK_VIEWER_INTERACTOR_H
#define QTOOLS_VTK_VIEWER_INTERACTOR_H

/**
 * Provides an interface to a VtkActorViewer via VTK events on the underlying viewer.
 * A single VtkViewerInteractor instance can be active on multiple viewers.
 * VtkViewerInteractorManager is used to coordinate attaching/detaching.
 *
 * Interactors are enabled by default but their enabled state can be changed dynamically.
 * If disabled, implementations of the virtual functions will not be called.
 */

#include "KeyPressHandler.h"
#include <vtkProp3D.h>

namespace QTools {

class VtkActorViewer;

class QTools_EXPORT VtkViewerInteractor
{
public:
    VtkViewerInteractor() : _enabled(true) {}
    virtual ~VtkViewerInteractor(){}

    // Temporarily disable calls to the virtual functions.
    void setEnabled( bool v) { _enabled = v;}
    bool isEnabled() const { return _enabled;}

    virtual QTools::KeyPressHandler* keyPressHandler() { return nullptr;}  // Not required.

    // Mouse enter/leave the viewer bounds.
    virtual void mouseLeave( const VtkActorViewer*){}
    virtual void mouseEnter( const VtkActorViewer*){}

    // Ongoing function calls are made at the event polling rate used by VTK.
    virtual void cameraStart(){}    // Immediately BEFORE camera movement begins.
    virtual void cameraRotate(){}   // Ongoing
    virtual void cameraDolly(){}    // Ongoing
    virtual void cameraSpin(){}     // Ongoing
    virtual void cameraPan(){}      // Ongoing
    virtual void cameraMove(){}     // Ongoing non-specific movement
    virtual void cameraStop(){}     // Immediately AFTER camera movement ends.

    // Similar for actors.
    virtual void actorStart( const vtkProp3D*){}
    virtual void actorRotate( const vtkProp3D*){}
    virtual void actorDolly( const vtkProp3D*){}
    virtual void actorSpin( const vtkProp3D*){}
    virtual void actorPan( const vtkProp3D*){}
    virtual void actorMove( const vtkProp3D*){}
    virtual void actorStop( const vtkProp3D*){}

private:
    bool _enabled;
};  // end class

using VVI = VtkViewerInteractor;

}   // end namespace

#endif
