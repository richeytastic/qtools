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
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
using QTools::VtkViewerSwitchInteractor;

vtkStandardNewMacro( VtkViewerSwitchInteractor)

VtkViewerSwitchInteractor::VtkViewerSwitchInteractor()
    : _iman(nullptr), _isel(nullptr), _iuse(nullptr), _useCamOffAct(false)
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


void VtkViewerSwitchInteractor::setTrackballActor() { _iuse = _isel = _iact;}
void VtkViewerSwitchInteractor::setTrackballCamera() { _iuse = _isel = _icam;}


void VtkViewerSwitchInteractor::OnLeftButtonDown()
{
    this->GrabFocus(this->EventCallbackCommand);
    if ( !_iman->doOnLeftButtonDown())
    {
        if ( !_iman->isInteractionLocked())
        {
            if ( GetInteractor()->GetControlKey())
                setStartState( VTKIS_DOLLY);
            else if ( GetInteractor()->GetShiftKey())
                setStartState( VTKIS_PAN);
            else
                setStartState( VTKIS_ROTATE);
        }   // end if
    }   // end if
}   // end OnLeftButtonDown


void VtkViewerSwitchInteractor::OnLeftButtonUp()
{
    if ( !_iman->doOnLeftButtonUp())
    {
        if ( !_iman->isInteractionLocked())
            setEndState();
    }   // end if
}   // end OnLeftButtonUp


void VtkViewerSwitchInteractor::OnRightButtonDown()
{
    this->GrabFocus(this->EventCallbackCommand);
    if ( !_iman->doOnRightButtonDown())
    {
        if ( !_iman->isInteractionLocked())
        {
            if ( GetInteractor()->GetControlKey())
                setStartState( VTKIS_DOLLY);
            else
                setStartState( VTKIS_PAN);
        }   // end if
    }   // end if
}   // end OnRightButtonDown


void VtkViewerSwitchInteractor::OnRightButtonUp()
{
    if ( !_iman->doOnRightButtonUp())
    {
        if ( !_iman->isInteractionLocked())
            setEndState();
    }   // end if
}   // end OnRightButtonUp


void VtkViewerSwitchInteractor::OnMiddleButtonDown()
{
    this->GrabFocus(this->EventCallbackCommand);
    if ( !_iman->doOnMiddleButtonDown())
    {
        if ( !_iman->isInteractionLocked())
            setStartState( VTKIS_DOLLY);
    }   // end if
}   // end OnMiddleButtonDown


void VtkViewerSwitchInteractor::OnMiddleButtonUp()
{
    if ( !_iman->doOnMiddleButtonUp())
    {
        if ( !_iman->isInteractionLocked())
            setEndState();
    }   // end if
}   // end OnMiddleButtonUp


void VtkViewerSwitchInteractor::OnMouseWheelForward()
{
    this->GrabFocus(this->EventCallbackCommand);
    if ( !_iman->doOnMouseWheelForward())
    {
        if ( !_iman->isInteractionLocked())
            doChunkAction( VTKIS_DOLLY, [this](){ doDolly( pow(1.1, _iuse->GetMouseWheelMotionFactor()));});
    }   // end if
}   // end OnMouseWheelForward


void VtkViewerSwitchInteractor::OnMouseWheelBackward()
{
    this->GrabFocus(this->EventCallbackCommand);
    if ( !_iman->doOnMouseWheelBackward())
    {
        if ( !_iman->isInteractionLocked())
            doChunkAction( VTKIS_DOLLY, [this](){ doDolly( pow(1.1, -_iuse->GetMouseWheelMotionFactor()));});
    }   // end if
}   // end OnMouseWheelBackward


// private
void VtkViewerSwitchInteractor::setStartState( int nstate)
{
    vtkRenderWindowInteractor *rwi = GetInteractor();
    const int x = rwi->GetEventPosition()[0];
    const int y = rwi->GetEventPosition()[1];

    if ( _isel == _iact)
    {
        _iact->SetInteractor(rwi);
        _iact->FindPokedRenderer(x,y);
        _iuse = _iact;
    }   // end if

    if ( _isel == _icam || (_useCamOffAct && !_iact->findPickedActor(x,y)))
    {
        _icam->SetInteractor(rwi);
        _icam->FindPokedRenderer(x,y);
        _iuse = _icam;
    }   // end if

    _iuse->StartState( nstate);

    if ( _iuse == _iact)
        _iman->doBeforeActorStart( _iact->prop());
    else
        _iman->doBeforeCameraStart();
}   // end setStartState


// private 
void VtkViewerSwitchInteractor::setEndState()
{
    _iuse->SetInteractor(GetInteractor());
    _iuse->StopState();
    _iuse->SetInteractor(nullptr);

    if ( _iuse == _iact)
        _iman->doAfterActorStop( _iact->prop());
    else
        _iman->doAfterCameraStop();

    _iuse = _isel;
}   // end setEndState


// private
void VtkViewerSwitchInteractor::doChunkAction( int nstate, std::function<void()> chunkFn)
{
    setStartState(nstate);
    chunkFn();
    setEndState();
}   // end doChunkAction


// private
void VtkViewerSwitchInteractor::doDolly( double factor)
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
}   // end doDolly


// private
void VtkViewerSwitchInteractor::doDolly()
{
    _iuse->Dolly();
    if ( _iuse == _iact)
        _iman->doAfterActorDolly( _iact->prop());
    else
        _iman->doAfterCameraDolly();
}   // end doDolly


// private
void VtkViewerSwitchInteractor::doRotate()
{
    _iuse->Rotate();
    if ( _iuse == _iact)
        _iman->doAfterActorRotate( _iact->prop());
    else
        _iman->doAfterCameraRotate();
}   // end doRotate


// private
void VtkViewerSwitchInteractor::doPan()
{
    _iuse->Pan();
    if ( _iuse == _iact)
        _iman->doAfterActorPan( _iact->prop());
    else
        _iman->doAfterCameraPan();
}   // end doPan


// private
void VtkViewerSwitchInteractor::doSpin()
{
    _iuse->Spin();
    if ( _iuse == _iact)
        _iman->doAfterActorSpin( _iact->prop());
    else
        _iman->doAfterCameraSpin();
}   // end doSpin


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
            _iuse->InvokeEvent( vtkCommand::InteractionEvent, nullptr);

        _iuse->SetInteractor(nullptr);
    }   // end if
}   // end OnMouseMove


void VtkViewerSwitchInteractor::OnEnter() { _iman->doOnEnter();}
void VtkViewerSwitchInteractor::OnLeave() { _iman->doOnLeave();}
