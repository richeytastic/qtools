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

/**
 * Handle key press events on QVTKWidget descendent VtkActorViewer.
 */

#ifndef QTOOLS_KEY_PRESS_HANDLER_H
#define QTOOLS_KEY_PRESS_HANDLER_H

#include <QKeyEvent>
#include "QTools_Export.h"

namespace QTools
{

class VtkActorViewer;

class QTools_EXPORT KeyPressHandler
{
protected:
    // Return true if key event is handled. Note that this function
    // allows for QKeyEvents having type of both KeyPress and KeyRelease.
    virtual bool handleKeyPress( QKeyEvent*) = 0;
    friend class VtkActorViewer;
};	// end class

}	// end namespace

#endif
