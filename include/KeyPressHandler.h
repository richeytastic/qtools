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

#ifndef QTOOLS_KEY_PRESS_HANDLER_H
#define QTOOLS_KEY_PRESS_HANDLER_H

#include "QTools_Export.h"
#include <QKeyEvent>

namespace QTools {
class VtkViewerInteractor;

class QTools_EXPORT KeyPressHandler
{
public:
    // If instantiated by a VtkViewerInteractor, it should pass itself in to the
    // constructor so this KeyPressHandler can query VtkViewerInteractor::isEnabled
    // when deciding whether to process key press events.
    explicit KeyPressHandler( const VtkViewerInteractor* vvi=nullptr);

    // These functions called by VtkActorViewer.
    bool handleKeyPress( QKeyEvent*);
    bool handleKeyRelease( QKeyEvent*);

protected:
    // Deriving classes should override one or both of these functions which are
    // only ever called if the instantiating VtkViewerInteractor (if it exists) is
    // enabled. Return true if key event is handled.
    virtual bool doHandleKeyPress( QKeyEvent*) { return false;}
    virtual bool doHandleKeyRelease( QKeyEvent*) { return false;}

private:
    const VtkViewerInteractor *_vvi;
};	// end class

}	// end namespace

#endif
