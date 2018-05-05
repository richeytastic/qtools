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
 * Multiple VtkViewerInteractor instances can be active on a single viewer at once,
 * but a VtkViewerInteractor can only be attached to a single VtkActorViewer.
 * VtkViewerInteractorManager is used to coordinate attaching/detaching.
 * Although not requiring to be a QObject type itself, VtkViewerInteractor
 * derives from QObject since most derived types of VtkViewerInteractor will
 * want to define signals to communicate user interactions to other code.
 */

#include "KeyPressHandler.h"

namespace QTools {

class QTools_EXPORT VtkViewerInteractor : public QObject
{ Q_OBJECT
public:
    VtkViewerInteractor(){}
    virtual ~VtkViewerInteractor(){}

    virtual void mouseMove( const QPoint&){}  // Move mouse with no buttons depressed.
    virtual void leftDrag( const QPoint&){}   // Move mouse with left button depressed.
    virtual void rightDrag( const QPoint&){}  // Move mouse with right button depressed.
    virtual void middleDrag( const QPoint&){} // Move mouse with middle button depressed.

    // Mouse enter/leave the viewer bounds.
    virtual void mouseLeave( const QPoint&){}
    virtual void mouseEnter( const QPoint&){}

    virtual void mouseWheelForward( const QPoint&){}
    virtual void mouseWheelBackward( const QPoint&){}

    virtual void middleButtonDown( const QPoint&){}
    virtual void middleButtonUp( const QPoint&){}

    virtual void rightButtonDown( const QPoint&){}
    virtual void rightButtonUp( const QPoint&){}

    virtual void leftButtonDown( const QPoint&){}  // Not called if leftDoubleClick
    virtual void leftButtonUp( const QPoint&){}    // Not called if leftDoubleClick
    virtual void leftDoubleClick( const QPoint&){}

    virtual QTools::KeyPressHandler* keyPressHandler() { return NULL;}  // Not required.

private:
    VtkViewerInteractor( const VtkViewerInteractor&);   // No copy
    void operator=( const VtkViewerInteractor&);        // No copy
};  // end class


typedef VtkViewerInteractor VVI;

}   // end namespace

#endif
