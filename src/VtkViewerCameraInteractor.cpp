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

#include <VtkViewerCameraInteractor.h>
#include <VtkViewerInteractorManager.h>   // For _delegate calls
#include <vtkObjectFactory.h>   // vtkStandardNewMacro
using QTools::VtkViewerCameraInteractor;

vtkStandardNewMacro( VtkViewerCameraInteractor)

VtkViewerCameraInteractor::VtkViewerCameraInteractor() {}

void VtkViewerCameraInteractor::OnLeftButtonDown()
{
    _delegate->doOnLeftButtonDown();
    if ( !_delegate->isInteractionLocked())
        vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}   // end OnLeftButtonDown


void VtkViewerCameraInteractor::OnLeftButtonUp()
{
    _delegate->doOnLeftButtonUp();
    if ( !_delegate->isInteractionLocked())
        vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
}   // end OnLeftButtonUp


void VtkViewerCameraInteractor::OnRightButtonDown()
{
    _delegate->doOnRightButtonDown();
    if ( !_delegate->isInteractionLocked())
        vtkInteractorStyleTrackballCamera::OnMiddleButtonDown();    // Want the right mouse button to pan
}   // end OnRightButtonDown


void VtkViewerCameraInteractor::OnRightButtonUp()
{
    _delegate->doOnRightButtonUp();
    if ( !_delegate->isInteractionLocked())
        vtkInteractorStyleTrackballCamera::OnMiddleButtonUp();  // Right mouse pans
}   // end OnRightButtonUp


// Don't pass middle mouse button events up to TrackballCamera since we want right mouse button to pan
void VtkViewerCameraInteractor::OnMiddleButtonDown() { _delegate->doOnMiddleButtonDown();}
void VtkViewerCameraInteractor::OnMiddleButtonUp() { _delegate->doOnMiddleButtonUp();}


void VtkViewerCameraInteractor::OnMouseWheelForward()
{
    _delegate->doOnMouseWheelForward();
    if ( !_delegate->isInteractionLocked())
        vtkInteractorStyleTrackballCamera::OnMouseWheelForward();   // FoV Zoom
}   // end OnMouseWheelForward

void VtkViewerCameraInteractor::OnMouseWheelBackward()
{
    _delegate->doOnMouseWheelBackward();
    if ( !_delegate->isInteractionLocked())
        vtkInteractorStyleTrackballCamera::OnMouseWheelBackward();  // FoV Zoom
}   // end OnMouseWheelBackward


void VtkViewerCameraInteractor::OnMouseMove()
{
    _delegate->doOnMouseMove();
    if ( !_delegate->isInteractionLocked())
        vtkInteractorStyleTrackballCamera::OnMouseMove();
}   // end OnMouseMove


// vtkInteractorStyleTrackballCamera does not implement OnEnter/Leave
void VtkViewerCameraInteractor::OnEnter() { _delegate->doOnEnter();}
void VtkViewerCameraInteractor::OnLeave() { _delegate->doOnLeave();}
