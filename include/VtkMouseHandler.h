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

#ifndef QTOOLS_VTK_MOUSE_HANDLER_H
#define QTOOLS_VTK_MOUSE_HANDLER_H

/**
 * If returning true from any of the virtual functions, the respective mouse input
 * will not be passed through to the viewer's underlying camera/actor moving routines.
 */

#include "QTools_Export.h"

namespace QTools {

class VtkActorViewer;

class QTools_EXPORT VtkMouseHandler
{
public:
    VtkMouseHandler() : _enabled(true) {}
    virtual ~VtkMouseHandler() {}

    // Temporarily disable calls to the virtual functions.
    void setEnabled( bool v) { _enabled = v;}
    bool isEnabled() const { return _enabled;}

    virtual void mouseEnter( const VtkActorViewer*){}
    virtual void mouseLeave( const VtkActorViewer*){}

    virtual bool mouseMove(){ return false;}  // Move mouse with no buttons depressed.

    virtual bool leftButtonDown(){ return false;}  // Not called if leftDoubleClick
    virtual bool leftButtonUp(){ return false;}    // Not called if leftDoubleClick
    virtual bool leftDoubleClick(){ return false;}

    virtual bool middleButtonDown(){ return false;}
    virtual bool middleButtonUp(){ return false;}

    virtual bool rightButtonDown(){ return false;}
    virtual bool rightButtonUp(){ return false;}

    virtual bool leftDrag(){ return false;}   // Move mouse with left button depressed.
    virtual bool rightDrag(){ return false;}  // Move mouse with right button depressed.
    virtual bool middleDrag(){ return false;} // Move mouse with middle button depressed.

    virtual bool mouseWheelForward(){ return false;}
    virtual bool mouseWheelBackward(){ return false;}

private:
    bool _enabled;
};  // end class

using VMH = VtkMouseHandler;

}   // end namespace

#endif
