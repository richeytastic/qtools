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

#include <VtkViewerActorInteractor.h>
#include <VtkViewerInteractorManager.h>   // For _iman calls
#include <vtkObjectFactory.h>   // vtkStandardNewMacro
using QTools::VtkViewerActorInteractor;

vtkStandardNewMacro( VtkViewerActorInteractor)

VtkViewerActorInteractor::VtkViewerActorInteractor() {}

void VtkViewerActorInteractor::OnLeftButtonDown()
{
    if ( _iman->doOnLeftButtonDown())
        return;
    if ( !_iman->isInteractionLocked())
        vtkInteractorStyleTrackballActor::OnLeftButtonDown();
}   // end OnLeftButtonDown


void VtkViewerActorInteractor::OnLeftButtonUp()
{
    if ( _iman->doOnLeftButtonUp())
        return;
    if ( !_iman->isInteractionLocked())
        vtkInteractorStyleTrackballActor::OnLeftButtonUp();
}   // end OnLeftButtonUp


void VtkViewerActorInteractor::OnRightButtonDown()
{
    if ( _iman->doOnRightButtonDown())
        return;
    if ( !_iman->isInteractionLocked())
        vtkInteractorStyleTrackballActor::OnMiddleButtonDown();    // Want the right mouse button to pan
}   // end OnRightButtonDown


void VtkViewerActorInteractor::OnRightButtonUp()
{
    if ( _iman->doOnRightButtonUp())
        return;
    if ( !_iman->isInteractionLocked())
        vtkInteractorStyleTrackballActor::OnMiddleButtonUp();  // Right mouse pans
}   // end OnRightButtonUp


// Don't pass middle mouse button events up to TrackballActor since we want right mouse button to pan
void VtkViewerActorInteractor::OnMiddleButtonDown() { _iman->doOnMiddleButtonDown();}
void VtkViewerActorInteractor::OnMiddleButtonUp() { _iman->doOnMiddleButtonUp();}


// vtkInteractorStyleTrackballActor does not implement OnMouseWheelForward/Backward
void VtkViewerActorInteractor::OnMouseWheelForward() { _iman->doOnMouseWheelForward();}
void VtkViewerActorInteractor::OnMouseWheelBackward() { _iman->doOnMouseWheelBackward();}


void VtkViewerActorInteractor::OnMouseMove()
{
    if ( _iman->doOnMouseMove())
        return;
    if ( !_iman->isInteractionLocked())
        vtkInteractorStyleTrackballActor::OnMouseMove();
}   // end OnMouseMove


// vtkInteractorStyleTrackballActor does not implement OnEnter/Leave
void VtkViewerActorInteractor::OnEnter() { _iman->doOnEnter();}
void VtkViewerActorInteractor::OnLeave() { _iman->doOnLeave();}
