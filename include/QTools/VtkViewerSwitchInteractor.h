/************************************************************************
 * Copyright (C) 2020 Richard Palmer
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

#ifndef QTOOLS_VTK_VIEWER_SWITCH_INTERACTOR_H
#define QTOOLS_VTK_VIEWER_SWITCH_INTERACTOR_H

#include "QTools_Export.h"
#include <vtkNew.h>
#include "VtkViewerActorInteractor.h"
#include "VtkViewerCameraInteractor.h"
#include <functional>

namespace QTools {

class VtkViewerInteractorManager;

class QTools_EXPORT VtkViewerSwitchInteractor : public vtkInteractorStyle
{
public:
    static VtkViewerSwitchInteractor* New();
    vtkTypeMacro( VtkViewerSwitchInteractor, vtkInteractorStyle)

    void SetAutoAdjustCameraClippingRange( int) override;

    void OnChar() override {}   // Override VTK key press handling (quit, pick etc)

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

    // Set mode to be actor interaction. If the allowedProp is null then any actor
    // will be interacted with. If not null, only the specified prop will be interacted with.
    void setTrackballActor( const vtkProp3D *allowedProp=nullptr);

    void setTrackballCamera();

    void SetCurrentRenderer(vtkRenderer*) override;

protected:
    VtkViewerSwitchInteractor();

private:
    vtkNew<VtkViewerActorInteractor> _iact;
    vtkNew<VtkViewerCameraInteractor> _icam;
    VtkViewerInteractorManager *_iman;
    vtkInteractorStyle *_isel, *_iuse;
    const vtkProp *_allowedProp;

    void _setUseInteractor();
    void _setStartState(int);
    void _setEndState();
    void _doChunkAction(int, std::function<void()>);
    void _doDolly(double);
    void _doDolly();
    void _doRotate();
    void _doSpin();
    void _doPan();

    VtkViewerSwitchInteractor( const VtkViewerSwitchInteractor&) = delete;
    void operator=( const VtkViewerSwitchInteractor&) = delete;
};  // end class

}   // end namespace

#endif
