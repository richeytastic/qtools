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

#include <VtkViewerSwitchInteractor.h>
#include <VtkViewerInteractorManager.h>   // For _iman calls
#include <vtkObjectFactory.h>   // vtkStandardNewMacro
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
using QTools::VtkViewerSwitchInteractor;

vtkStandardNewMacro( VtkViewerSwitchInteractor)

VtkViewerSwitchInteractor::VtkViewerSwitchInteractor()
    : _iman(nullptr),
      _iactor( QTools::VtkViewerActorInteractor::New()),
      _icamera( QTools::VtkViewerCameraInteractor::New()),
      _istyle(nullptr)
{
    setTrackballCamera();   // Default
}   // end ctor


VtkViewerSwitchInteractor::~VtkViewerSwitchInteractor()
{
    _iactor->Delete();
    _icamera->Delete();
}   // end dtor


void VtkViewerSwitchInteractor::SetAutoAdjustCameraClippingRange( int v)
{
    this->AutoAdjustCameraClippingRange = v;
    _iactor->SetAutoAdjustCameraClippingRange(v);
    _icamera->SetAutoAdjustCameraClippingRange(v);
    this->Modified();
}   // end SetAutoAdjustCameraClippingRange


void VtkViewerSwitchInteractor::SetDefaultRenderer( vtkRenderer* renderer)
{
    vtkInteractorStyle::SetDefaultRenderer(renderer);
    _iactor->SetDefaultRenderer(renderer);
    _icamera->SetDefaultRenderer(renderer);
}   // end SetDefaultRenderer


void VtkViewerSwitchInteractor::SetCurrentRenderer( vtkRenderer* renderer)
{
    vtkInteractorStyle::SetCurrentRenderer(renderer);
    _iactor->SetCurrentRenderer(renderer);
    _icamera->SetCurrentRenderer(renderer);
}   // end SetCurrentRenderer


void VtkViewerSwitchInteractor::setTrackballActor() { _istyle = _iactor;}
void VtkViewerSwitchInteractor::setTrackballCamera() { _istyle = _icamera;} 


// private
void VtkViewerSwitchInteractor::doStartState( int nstate)
{
    vtkRenderWindowInteractor *rwi = GetInteractor();
    const int x = rwi->GetEventPosition()[0];
    const int y = rwi->GetEventPosition()[1];

    _istyle->SetInteractor( rwi);
    _istyle->FindPokedRenderer(x,y);
    if ( _istyle == _iactor)
        _iactor->findPickedActor(x,y);
    _istyle->StartState(nstate);
    _istyle->SetInteractor(nullptr);    // Ensure VtkViewerSwitchInteractor receives follow-up interactions
}   // end doStartState


// private 
void VtkViewerSwitchInteractor::doEndState()
{
    int fstate = _istyle->GetState();
    _istyle->SetInteractor(GetInteractor());
    _istyle->StopState();
    _istyle->SetInteractor(nullptr);

    if ( fstate != VTKIS_NONE && fstate != VTKIS_START)
    {
        if ( _istyle == _icamera)
            _iman->doAfterCameraStop();
        else
            _iman->doAfterActorStop();
    }   // end if
}   // end doEndState


void VtkViewerSwitchInteractor::OnLeftButtonDown()
{
    this->GrabFocus(this->EventCallbackCommand);
    if ( _iman->doOnLeftButtonDown())
        return;

    if ( !_iman->isInteractionLocked())
    {
        if ( GetInteractor()->GetControlKey())
            doStartState( VTKIS_DOLLY);
        else if ( GetInteractor()->GetShiftKey())
            doStartState( VTKIS_PAN);
        else
            doStartState( VTKIS_ROTATE);
    }   // end if
}   // end OnLeftButtonDown


void VtkViewerSwitchInteractor::OnLeftButtonUp()
{
    if ( _iman->doOnLeftButtonUp())
        return;

    if ( !_iman->isInteractionLocked())
        doEndState();
    this->ReleaseFocus();
}   // end OnLeftButtonUp


void VtkViewerSwitchInteractor::OnRightButtonDown()
{
    this->GrabFocus(this->EventCallbackCommand);
    if ( _iman->doOnRightButtonDown())
        return;
    if ( !_iman->isInteractionLocked())
    {
        if ( GetInteractor()->GetControlKey())
            doStartState( VTKIS_DOLLY);
        else
            doStartState( VTKIS_PAN);
    }   // end if
}   // end OnRightButtonDown


void VtkViewerSwitchInteractor::OnRightButtonUp()
{
    if ( _iman->doOnRightButtonUp())
        return;
    if ( !_iman->isInteractionLocked())
        doEndState();
    this->ReleaseFocus();
}   // end OnRightButtonUp


void VtkViewerSwitchInteractor::OnMiddleButtonDown()
{
    this->GrabFocus(this->EventCallbackCommand);
    if ( _iman->doOnMiddleButtonDown())
        return;
    if ( !_iman->isInteractionLocked())
        doStartState( VTKIS_DOLLY);
}   // end OnMiddleButtonDown


void VtkViewerSwitchInteractor::OnMiddleButtonUp()
{
    if ( _iman->doOnMiddleButtonUp())
        return;
    if ( !_iman->isInteractionLocked())
        doEndState();
    this->ReleaseFocus();
}   // end OnMiddleButtonUp


void VtkViewerSwitchInteractor::OnMouseWheelForward()
{
    if ( _iman->doOnMouseWheelForward())
        return;
    if ( !_iman->isInteractionLocked())
        doChunkAction( VTKIS_DOLLY, [this](){ doDolly( pow(1.1, _istyle->GetMouseWheelMotionFactor()));});
}   // end OnMouseWheelForward


void VtkViewerSwitchInteractor::OnMouseWheelBackward()
{
    if ( _iman->doOnMouseWheelBackward())
        return;
    if ( !_iman->isInteractionLocked())
        doChunkAction( VTKIS_DOLLY, [this](){ doDolly( pow(1.1, -_istyle->GetMouseWheelMotionFactor()));});
}   // end OnMouseWheelBackward


// private
void VtkViewerSwitchInteractor::doChunkAction( int nstate, std::function<void()> chunkFn)
{
    vtkRenderWindowInteractor *rwi = GetInteractor();
    const int x = rwi->GetEventPosition()[0];
    const int y = rwi->GetEventPosition()[1];
    _istyle->SetInteractor(rwi);
    _istyle->FindPokedRenderer(x,y);
    if ( _istyle == _iactor)
        _iactor->findPickedActor(x,y);
    _istyle->StartState( nstate);

    chunkFn();

    _istyle->StopState();
    _istyle->SetInteractor(nullptr);

    if ( _istyle == _icamera)
        _iman->doAfterCameraStop();
    else
        _iman->doAfterActorStop();
}   // end doChunkAction


// private
void VtkViewerSwitchInteractor::doDolly( double factor)
{
    if ( _istyle == _icamera)
    {
        _icamera->dolly( factor);
        _iman->doAfterCameraDolly();
    }   // end if
    else
    {
        _iactor->dolly( factor);
        _iman->doAfterActorDolly();
    }   // end else
}   // end doDolly


// private
void VtkViewerSwitchInteractor::doDolly()
{
    _istyle->Dolly();
    if ( _istyle == _icamera)
        _iman->doAfterCameraDolly();
    else
        _iman->doAfterActorDolly();
}   // end doDolly


// private
void VtkViewerSwitchInteractor::doRotate()
{
    _istyle->Rotate();
    if ( _istyle == _icamera)
        _iman->doAfterCameraRotate();
    else
        _iman->doAfterActorRotate();
}   // end doRotate


// private
void VtkViewerSwitchInteractor::doPan()
{
    _istyle->Pan();
    if ( _istyle == _icamera)
        _iman->doAfterCameraPan();
    else
        _iman->doAfterActorPan();
}   // end doPan


// private
void VtkViewerSwitchInteractor::doSpin()
{
    _istyle->Spin();
    if ( _istyle == _icamera)
        _iman->doAfterCameraSpin();
    else
        _iman->doAfterActorSpin();
}   // end doSpin


void VtkViewerSwitchInteractor::OnMouseMove()
{
    if ( _iman->doOnMouseMove())
        return;
    if ( !_iman->isInteractionLocked())
    {
        vtkRenderWindowInteractor *rwi = GetInteractor();
        _istyle->SetInteractor(rwi);
        const int x = rwi->GetEventPosition()[0];
        const int y = rwi->GetEventPosition()[1];
        _istyle->FindPokedRenderer(x,y);

        bool workedState = false;
        switch ( _istyle->GetState())
        {
            case VTKIS_ROTATE:
                doRotate();
                workedState = true;
                break;
            case VTKIS_PAN:
                doPan();
                workedState = true;
                break;
            case VTKIS_DOLLY:
                doDolly();
                workedState = true;
                break;
            case VTKIS_SPIN:
                doSpin();
                workedState = true;
                break;
        }   // end switch

        if ( workedState)
            _istyle->InvokeEvent( vtkCommand::InteractionEvent, nullptr);

        _istyle->SetInteractor(nullptr);
    }   // end if
}   // end OnMouseMove


void VtkViewerSwitchInteractor::OnEnter() { _iman->doOnEnter();}
void VtkViewerSwitchInteractor::OnLeave() { _iman->doOnLeave();}
