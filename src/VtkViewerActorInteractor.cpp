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
#include <vtkObjectFactory.h>   // vtkStandardNewMacro
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkProp3D.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
using QTools::VtkViewerActorInteractor;

vtkStandardNewMacro( VtkViewerActorInteractor)

VtkViewerActorInteractor::VtkViewerActorInteractor() {}
VtkViewerActorInteractor::~VtkViewerActorInteractor() {}


bool VtkViewerActorInteractor::findPickedActor( int x, int y)
{
    FindPickedActor(x,y);   // Protected
    return isOnActor();
}   // findPickedActor


bool VtkViewerActorInteractor::isOnActor() const { return prop() != nullptr;}


const vtkProp3D* VtkViewerActorInteractor::prop() const { return this->InteractionProp;}


// Source code copied and edited from vtkInteractorStyleTrackballActor.cxx
void VtkViewerActorInteractor::dolly( double dollyFactor)
{
    if (!this->CurrentRenderer || !this->InteractionProp)
        return;

    vtkRenderWindowInteractor *rwi = this->Interactor;
    vtkCamera *cam = this->CurrentRenderer->GetActiveCamera();

    double view_point[3], view_focus[3];
    cam->GetPosition(view_point);
    cam->GetFocalPoint(view_focus);

    dollyFactor -= 1.0;

    double motion_vector[3];
    motion_vector[0] = (view_point[0] - view_focus[0]) * dollyFactor;
    motion_vector[1] = (view_point[1] - view_focus[1]) * dollyFactor;
    motion_vector[2] = (view_point[2] - view_focus[2]) * dollyFactor;

    if ( this->InteractionProp->GetUserMatrix())
    {
        vtkTransform *t = vtkTransform::New();
        t->PostMultiply();
        t->SetMatrix( this->InteractionProp->GetUserMatrix());
        t->Translate( motion_vector[0], motion_vector[1], motion_vector[2]);
        this->InteractionProp->GetUserMatrix()->DeepCopy( t->GetMatrix());
        t->Delete();
    } // end if
    else
        this->InteractionProp->AddPosition(motion_vector);

    if (this->AutoAdjustCameraClippingRange)
        this->CurrentRenderer->ResetCameraClippingRange();

    rwi->Render();
}   // end dolly

