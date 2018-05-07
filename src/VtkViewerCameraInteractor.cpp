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
#include <VtkViewerInteractorManager.h>   // For _iman calls
#include <vtkObjectFactory.h>   // vtkStandardNewMacro
using QTools::VtkViewerCameraInteractor;

vtkStandardNewMacro( VtkViewerCameraInteractor)

VtkViewerCameraInteractor::VtkViewerCameraInteractor() {}

void VtkViewerCameraInteractor::OnLeftButtonDown()
{
    if ( _iman->doOnLeftButtonDown())
        return;
    if ( !_iman->isInteractionLocked())
        vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}   // end OnLeftButtonDown


void VtkViewerCameraInteractor::OnLeftButtonUp()
{
    if ( _iman->doOnLeftButtonUp())
        return;
    if ( !_iman->isInteractionLocked())
        vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
}   // end OnLeftButtonUp


void VtkViewerCameraInteractor::OnRightButtonDown()
{
    if ( _iman->doOnRightButtonDown())
        return;
    if ( !_iman->isInteractionLocked())
        vtkInteractorStyleTrackballCamera::OnMiddleButtonDown();    // Want the right mouse button to pan
}   // end OnRightButtonDown


void VtkViewerCameraInteractor::OnRightButtonUp()
{
    if ( _iman->doOnRightButtonUp())
        return;
    if ( !_iman->isInteractionLocked())
        vtkInteractorStyleTrackballCamera::OnMiddleButtonUp();  // Right mouse pans
}   // end OnRightButtonUp


// Don't pass middle mouse button events up to TrackballCamera since we want right mouse button to pan
void VtkViewerCameraInteractor::OnMiddleButtonDown() { _iman->doOnMiddleButtonDown();}
void VtkViewerCameraInteractor::OnMiddleButtonUp() { _iman->doOnMiddleButtonUp();}


void VtkViewerCameraInteractor::OnMouseWheelForward()
{
    if ( _iman->doOnMouseWheelForward())
        return;
    if ( !_iman->isInteractionLocked())
        vtkInteractorStyleTrackballCamera::OnMouseWheelForward();   // FoV Zoom
}   // end OnMouseWheelForward

void VtkViewerCameraInteractor::OnMouseWheelBackward()
{
    if ( _iman->doOnMouseWheelBackward())
        return;
    if ( !_iman->isInteractionLocked())
        vtkInteractorStyleTrackballCamera::OnMouseWheelBackward();  // FoV Zoom
}   // end OnMouseWheelBackward


void VtkViewerCameraInteractor::OnMouseMove()
{
    if ( _iman->doOnMouseMove())
        return;
    if ( !_iman->isInteractionLocked())
        vtkInteractorStyleTrackballCamera::OnMouseMove();
}   // end OnMouseMove


// vtkInteractorStyleTrackballCamera does not implement OnEnter/Leave
void VtkViewerCameraInteractor::OnEnter() { _iman->doOnEnter();}
void VtkViewerCameraInteractor::OnLeave() { _iman->doOnLeave();}
