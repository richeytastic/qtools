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

#include <VtkScalingActor.h>
#include <vtkProperty.h>
#include <VtkTools.h>       // RVTK
#include <Transformer.h>    // RFeatures
#include <cassert>
using QTools::VtkScalingActor;


VtkScalingActor::VtkScalingActor( vtkPolyDataAlgorithm* src)
{
    _glyph->SetSourceConnection( src->GetOutputPort());
    _glyph->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "DistanceToCamera");

    // Glyph is scaled on rendering updates by result from DistanceToCamera calculation.
    _glyph->SetInputConnection(_d2cam->GetOutputPort());
    setFixedScale(false);

    setPosition( cv::Vec3f(0,0,0));

    // Create the actor
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->AddInputConnection( _glyph->GetOutputPort());
    mapper->SetScalarVisibility(false);
    _actor->SetMapper(mapper);

    // Ambient lighting only
    vtkProperty* property = _actor->GetProperty();
    property->SetAmbient(1.0);
    property->SetDiffuse(0.0);
    property->SetSpecular(0.0);
}   // end ctor


void VtkScalingActor::copyProperties( const VtkScalingActor& sa)
{
    setRenderer(sa.renderer());
    setFixedScale(sa.fixedScale());
    setPickable(sa.pickable());
    setScaleFactor(sa.scaleFactor());
    setVisible(sa.visible());
    setPosition(sa.position());
    setColour(sa.colour());
    setOpacity(sa.opacity());
    pokeTransform(sa._actor->GetMatrix());
}   // end copyProperties


void VtkScalingActor::setRenderer( vtkRenderer* ren) { _d2cam->SetRenderer( ren);}
vtkRenderer* VtkScalingActor::renderer() const { return _d2cam->GetRenderer();}

void VtkScalingActor::setFixedScale( bool v)
{
    if ( v)
        _glyph->SetScaleModeToScaleByScalar();
    else
        _glyph->SetScaleModeToDataScalingOff();
}   // end setFixedScale

bool VtkScalingActor::fixedScale() const { return _glyph->GetScaleMode() != VTK_DATA_SCALING_OFF;}

void VtkScalingActor::setScaleFactor( double f) { _glyph->SetScaleFactor(f);}
double VtkScalingActor::scaleFactor() const { return _glyph->GetScaleFactor();}

void VtkScalingActor::setPickable( bool v) { _actor->SetPickable(v);}
bool VtkScalingActor::pickable() const { return _actor->GetPickable();}

void VtkScalingActor::setVisible( bool v) { _actor->SetVisibility(v);}
bool VtkScalingActor::visible() const { return _actor->GetVisibility();}

void VtkScalingActor::setPosition( const cv::Vec3f& v)
{
    _pos = v;
    vtkNew<vtkPolyData> pointSet;
    vtkNew<vtkPoints> points;
    points->InsertPoint( 0, &std::vector<double>({v[0], v[1], v[2]})[0]);
    pointSet->SetPoints( points);
    _d2cam->SetInputData( pointSet);
}   // end setPosition


void VtkScalingActor::setColour( double r, double g, double b) { _actor->GetProperty()->SetColor( r, g, b);}
void VtkScalingActor::setColour( const double c[3]) { _actor->GetProperty()->SetColor( const_cast<double*>(c));}
const double* VtkScalingActor::colour() const { return _actor->GetProperty()->GetColor();}

void VtkScalingActor::setOpacity( double a) { _actor->GetProperty()->SetOpacity(a);}
double VtkScalingActor::opacity() const { return _actor->GetProperty()->GetOpacity();}


// Makes temporary updates to the actor.
void VtkScalingActor::pokeTransform( const vtkMatrix4x4* vm) { _actor->PokeMatrix( const_cast<vtkMatrix4x4*>(vm));}


// Update the actual position with the temporary changes to the actor's matrix.
void VtkScalingActor::fixTransform()
{
    cv::Vec3f pos = position();
    const RFeatures::Transformer mover( RVTK::toCV(_actor->GetMatrix()));
    mover.transform(pos);  // In-place
    setPosition(pos);
}   // end fixTransform

