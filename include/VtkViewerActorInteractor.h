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

#include "QTools_Export.h"
#include <vtkInteractorStyleTrackballActor.h>

namespace QTools {

class VtkViewerInteractorManager;

class QTools_EXPORT VtkViewerActorInteractor : public vtkInteractorStyleTrackballActor
{
public:
    static VtkViewerActorInteractor* New();
    vtkTypeMacro( VtkViewerActorInteractor, vtkInteractorStyleTrackballActor)

    void OnChar() override {}   // Override VTK key press handling

    void OnLeftButtonDown() override;
    void OnLeftButtonUp() override;
    void OnMiddleButtonDown() override;
    void OnMiddleButtonUp() override;
    void OnRightButtonDown() override;
    void OnRightButtonUp() override;
    void OnMouseWheelForward() override;
    void OnMouseWheelBackward() override;
    
    void OnMouseMove() override;
    void OnEnter() override;
    void OnLeave() override;

    void setDelegate( VtkViewerInteractorManager* d) { _iman = d;}

protected:
    VtkViewerActorInteractor();
    virtual ~VtkViewerActorInteractor(){}

private:
    VtkViewerInteractorManager *_iman;
};  // end class

}   // end namespace

#endif
