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

#ifndef QTOOLS_VTK_VIEWER_INTERACTOR_H
#define QTOOLS_VTK_VIEWER_INTERACTOR_H

/**
 * Provides an interface to a VtkActorViewer via VTK events on the underlying viewer.
 * Multiple VtkViewerInteractor instances can be active on a single viewer at once, but
 * a VtkViewerInteractor can only be attached to a single VtkActorViewer.
 * VtkViewerInteractorManager is used to coordinate attaching/detaching. Overridden
 * functions should return true if they do not want the underlying VTK camera or actor
 * interactor to process the mouse control.
 *
 * Interactors are enabled by default but their enabled state can be changed on the fly.
 * If disabled, implementations of user inputs will not be called until reenabled again.
 *
 * Although not requiring to be a QObject type itself, VtkViewerInteractor derives from
 * QObject since most derived types of VtkViewerInteractor will want to define signals
 * to communicate user interactions to other code.
 */

#include "KeyPressHandler.h"

namespace QTools {

class QTools_EXPORT VtkViewerInteractor : public QObject
{ Q_OBJECT
public:
    VtkViewerInteractor() : _enabled(true) {}
    virtual ~VtkViewerInteractor(){}

    // Temporarily disable calls to the virtual mouse interaction functions and key press handler.
    void setEnabled( bool v) { _enabled = v; onEnabledStateChanged(v);}
    bool isEnabled() const { return _enabled;}

    virtual bool mouseMove( const QPoint&){ return false;}  // Move mouse with no buttons depressed.
    virtual bool leftDrag( const QPoint&){ return false;}   // Move mouse with left button depressed.
    virtual bool rightDrag( const QPoint&){ return false;}  // Move mouse with right button depressed.
    virtual bool middleDrag( const QPoint&){ return false;} // Move mouse with middle button depressed.

    // Mouse enter/leave the viewer bounds.
    virtual bool mouseLeave( const QPoint&){ return false;}
    virtual bool mouseEnter( const QPoint&){ return false;}

    virtual bool mouseWheelForward( const QPoint&){ return false;}
    virtual bool mouseWheelBackward( const QPoint&){ return false;}

    virtual bool middleButtonDown( const QPoint&){ return false;}
    virtual bool middleButtonUp( const QPoint&){ return false;}

    virtual bool rightButtonDown( const QPoint&){ return false;}
    virtual bool rightButtonUp( const QPoint&){ return false;}

    virtual bool leftButtonDown( const QPoint&){ return false;}  // Not called if leftDoubleClick
    virtual bool leftButtonUp( const QPoint&){ return false;}    // Not called if leftDoubleClick
    virtual bool leftDoubleClick( const QPoint&){ return false;}

    virtual QTools::KeyPressHandler* keyPressHandler() { return nullptr;}  // Not required.

    // These functions called immediately AFTER processing VTK interaction on either the camera or an actor.
    virtual void cameraRotate(){}
    virtual void cameraDolly(){}
    virtual void cameraSpin(){}
    virtual void cameraPan(){}
    virtual void cameraStop(){} // Stopped interacting with the camera

    virtual void actorRotate(){}
    virtual void actorDolly(){}
    virtual void actorSpin(){}
    virtual void actorPan(){}
    virtual void actorStop(){}  // Stopped interacting with the actor

protected:
    virtual void onEnabledStateChanged( bool){}

private:
    bool _enabled;
    VtkViewerInteractor( const VtkViewerInteractor&) = delete;
    void operator=( const VtkViewerInteractor&) = delete;
};  // end class


typedef VtkViewerInteractor VVI;

}   // end namespace

#endif
