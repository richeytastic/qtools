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

#ifndef QTOOLS_VTK_VIEWER_ACTOR_INTERACTOR_H
#define QTOOLS_VTK_VIEWER_ACTOR_INTERACTOR_H

/**
 * Used by VtkViewerSwitchInteractor simply to allow actor picking to
 * be carried out so that actor movements won't return on null actor
 * from inside the vtkInteractorStyleTrackballActor's movement functions
 * (Rotate, Pan, Dolly etc).
 */

#include "QTools_Export.h"
#include <vtkInteractorStyleTrackballActor.h>

namespace QTools {

class QTools_EXPORT VtkViewerActorInteractor : public vtkInteractorStyleTrackballActor
{
public:
    static VtkViewerActorInteractor* New();
    vtkTypeMacro( VtkViewerActorInteractor, vtkInteractorStyleTrackballActor)

    void OnChar() override {}   // Override VTK key press handling

    // Returns true if a vtkProp3D picked.
    bool findPickedActor( int x, int y);

    // Same as return value from findPickedActor.
    bool isOnActor() const;

    const vtkProp3D* prop() const;

    void dolly( double);

protected:
    VtkViewerActorInteractor();
    ~VtkViewerActorInteractor() override;
};  // end class

}   // end namespace

#endif
