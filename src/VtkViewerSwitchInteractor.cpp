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

#include <VtkViewerSwitchInteractor.h>
#include <VtkViewerInteractorManager.h>   // For _iman calls
#include <vtkObjectFactory.h>   // vtkStandardNewMacro
#include <vtkCallbackCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
using QTools::VtkViewerSwitchInteractor;

vtkStandardNewMacro( VtkViewerSwitchInteractor)

VtkViewerSwitchInteractor::VtkViewerSwitchInteractor()
    : _iman(nullptr), _isel(nullptr), _iuse(nullptr), _allowedProp(nullptr)
{
    setTrackballCamera();   // Default
}   // end ctor


void VtkViewerSwitchInteractor::SetAutoAdjustCameraClippingRange( int v)
{
    this->AutoAdjustCameraClippingRange = v;
    _iact->SetAutoAdjustCameraClippingRange(v);
    _icam->SetAutoAdjustCameraClippingRange(v);
    this->Modified();
}   // end SetAutoAdjustCameraClippingRange


void VtkViewerSwitchInteractor::SetCurrentRenderer( vtkRenderer* renderer)
{
    vtkInteractorStyle::SetCurrentRenderer(renderer);
    _iact->SetCurrentRenderer(renderer);
    _icam->SetCurrentRenderer(renderer);
}   // end SetCurrentRenderer


void VtkViewerSwitchInteractor::setTrackballActor( const vtkProp3D *p)
{
    _allowedProp = p;
    _iuse = _isel = _iact;
}   // end setTrackballActor


void VtkViewerSwitchInteractor::setTrackballCamera()
{
    _iuse = _isel = _icam;
}   // end setTrackballCamera


void VtkViewerSwitchInteractor::OnLeftButtonDown()
{
    this->GrabFocus(this->EventCallbackCommand);
    if ( !_iman->doOnLeftButtonDown() && !_iman->isInteractionLocked())
    {
        if ( GetInteractor()->GetControlKey())
            _setStartState( VTKIS_DOLLY);
        else if ( GetInteractor()->GetShiftKey())
            _setStartState( VTKIS_PAN);
        else
            _setStartState( VTKIS_ROTATE);
    }   // end if
}   // end OnLeftButtonDown


void VtkViewerSwitchInteractor::OnLeftButtonUp()
{
    if ( !_iman->isInteractionLocked())
        _setEndState();
    _iman->doOnLeftButtonUp();
}   // end OnLeftButtonUp


void VtkViewerSwitchInteractor::OnRightButtonDown()
{
    this->GrabFocus(this->EventCallbackCommand);
    if ( !_iman->doOnRightButtonDown() && !_iman->isInteractionLocked())
    {
        if ( GetInteractor()->GetControlKey())
            _setStartState( VTKIS_DOLLY);
        else
            _setStartState( VTKIS_PAN);
    }   // end if
}   // end OnRightButtonDown


void VtkViewerSwitchInteractor::OnRightButtonUp()
{
    if ( !_iman->isInteractionLocked())
        _setEndState();
    _iman->doOnRightButtonUp();
}   // end OnRightButtonUp


void VtkViewerSwitchInteractor::OnMiddleButtonDown()
{
    this->GrabFocus(this->EventCallbackCommand);
    if ( !_iman->doOnMiddleButtonDown() && !_iman->isInteractionLocked())
        _setStartState( VTKIS_DOLLY);
}   // end OnMiddleButtonDown


void VtkViewerSwitchInteractor::OnMiddleButtonUp()
{
    if ( !_iman->isInteractionLocked())
        _setEndState();
    _iman->doOnMiddleButtonUp();
}   // end OnMiddleButtonUp


void VtkViewerSwitchInteractor::OnMouseWheelForward()
{
    this->GrabFocus(this->EventCallbackCommand);
    if ( !_iman->doOnMouseWheelForward() && !_iman->isInteractionLocked())
        _doChunkAction( VTKIS_DOLLY, [this](){ _doDolly( pow(1.1, _iuse->GetMouseWheelMotionFactor()));});
}   // end OnMouseWheelForward


void VtkViewerSwitchInteractor::OnMouseWheelBackward()
{
    this->GrabFocus(this->EventCallbackCommand);
    if ( !_iman->doOnMouseWheelBackward() && !_iman->isInteractionLocked())
        _doChunkAction( VTKIS_DOLLY, [this](){ _doDolly( pow(1.1, -_iuse->GetMouseWheelMotionFactor()));});
}   // end OnMouseWheelBackward


void VtkViewerSwitchInteractor::_setUseInteractor()
{
    vtkRenderWindowInteractor *rwi = GetInteractor();
    const int x = rwi->GetEventPosition()[0];
    const int y = rwi->GetEventPosition()[1];
    bool onProp = false;

    _iuse = _icam;

    if ( _isel == _iact)
    {
        _iact->SetInteractor(rwi);
        _iact->FindPokedRenderer(x,y);
        onProp = _iact->findPickedActor(x,y);   // Could be true if on any prop
        if ( _allowedProp != nullptr)
            onProp = _iact->prop() == _allowedProp;
        if ( onProp)
            _iuse = _iact;
    }   // end if

    if ( _iuse == _icam)
    {
        _icam->SetInteractor(rwi);
        _icam->FindPokedRenderer(x,y);
    }   // end if
}   // end _setUseInteractor


void VtkViewerSwitchInteractor::_setStartState( int nstate)
{
    _setUseInteractor();
    _iuse->StartState( nstate);
    if ( _iuse == _iact)
        _iman->doBeforeActorStart( _iact->prop());
    else
        _iman->doBeforeCameraStart();
}   // end _setStartState


void VtkViewerSwitchInteractor::_setEndState()
{
    _iuse->SetInteractor(GetInteractor());
    _iuse->StopState();
    _iuse->SetInteractor(nullptr);

    if ( _iuse == _iact)
        _iman->doAfterActorStop( _iact->prop());
    else
        _iman->doAfterCameraStop();
}   // end _setEndState


void VtkViewerSwitchInteractor::_doChunkAction( int nstate, std::function<void()> chunkFn)
{
    _setStartState(nstate);
    chunkFn();
    _setEndState();
}   // end _doChunkAction


void VtkViewerSwitchInteractor::_doDolly( double factor)
{
    if ( _iuse == _iact)
    {
        _iact->dolly( factor);
        _iman->doAfterActorDolly( _iact->prop());
    }   // end if
    else
    {
        _icam->dolly( factor);
        _iman->doAfterCameraDolly();
    }   // end else
}   // end _doDolly


void VtkViewerSwitchInteractor::_doDolly()
{
    _iuse->Dolly();
    if ( _iuse == _iact)
        _iman->doAfterActorDolly( _iact->prop());
    else
        _iman->doAfterCameraDolly();
}   // end _doDolly


void VtkViewerSwitchInteractor::_doRotate()
{
    _iuse->Rotate();
    if ( _iuse == _iact)
        _iman->doAfterActorRotate( _iact->prop());
    else
        _iman->doAfterCameraRotate();
}   // end _doRotate


void VtkViewerSwitchInteractor::_doPan()
{
    _iuse->Pan();
    if ( _iuse == _iact)
        _iman->doAfterActorPan( _iact->prop());
    else
        _iman->doAfterCameraPan();
}   // end _doPan


void VtkViewerSwitchInteractor::_doSpin()
{
    _iuse->Spin();
    if ( _iuse == _iact)
        _iman->doAfterActorSpin( _iact->prop());
    else
        _iman->doAfterCameraSpin();
}   // end _doSpin


void VtkViewerSwitchInteractor::OnMouseMove()
{
    if ( _iman->doOnMouseMove())
        return;
    if ( !_iman->isInteractionLocked())
    {
        vtkRenderWindowInteractor *rwi = GetInteractor();
        _iuse->SetInteractor(rwi);
        const int x = rwi->GetEventPosition()[0];
        const int y = rwi->GetEventPosition()[1];
        _iuse->FindPokedRenderer(x,y);

        bool workedState = false;
        switch ( _iuse->GetState())
        {
            case VTKIS_ROTATE:
                _doRotate();
                workedState = true;
                break;
            case VTKIS_PAN:
                _doPan();
                workedState = true;
                break;
            case VTKIS_DOLLY:
                _doDolly();
                workedState = true;
                break;
            case VTKIS_SPIN:
                _doSpin();
                workedState = true;
                break;
        }   // end switch

        if ( workedState)
            _iuse->InvokeEvent( vtkCommand::InteractionEvent, nullptr);

        _iuse->SetInteractor(nullptr);
    }   // end if
}   // end OnMouseMove


void VtkViewerSwitchInteractor::OnEnter() { _iman->doOnEnter();}
void VtkViewerSwitchInteractor::OnLeave() { _iman->doOnLeave();}
