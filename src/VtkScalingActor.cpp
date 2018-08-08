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
using QTools::VtkScalingActor;
using QTools::VtkActorViewer;


void VtkScalingActor::init( vtkPolyDataAlgorithm* src, const cv::Vec3f& pos)
{
    _viewer = nullptr;
    _glyph->SetSourceConnection( src->GetOutputPort());
    _glyph->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "DistanceToCamera");

    setPosition( pos);

    // Glyph is scaled on rendering updates by result from DistanceToCamera calculation.
    _glyph->SetInputConnection(_d2cam->GetOutputPort());
    setFixedScale( true);

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
}   // end init


VtkScalingActor::VtkScalingActor( vtkPolyDataAlgorithm* src, double x, double y, double z)
{
    const cv::Vec3f pos((float)x, (float)y, (float)z);
    init( src, pos);
}   // end ctor


VtkScalingActor::VtkScalingActor( vtkPolyDataAlgorithm* src, const cv::Vec3f& pos)
{
    init( src, pos);
}   // end ctor


void VtkScalingActor::setPickable( bool v) { _actor->SetPickable(v);}
bool VtkScalingActor::pickable() const { return _actor->GetPickable();}

void VtkScalingActor::setFixedScale( bool fixedScale)
{
    if ( fixedScale)
        _glyph->SetScaleModeToScaleByScalar();
    else
        _glyph->SetScaleModeToDataScalingOff();
}   // end setFixedScale

bool VtkScalingActor::fixedScale() const { return _glyph->GetScaleMode() != VTK_DATA_SCALING_OFF;}

void VtkScalingActor::setScaleFactor( double f) { _glyph->SetScaleFactor(f);}
double VtkScalingActor::scaleFactor() const { return _glyph->GetScaleFactor();}

void VtkScalingActor::setVisible( bool v) { _actor->SetVisibility(v);}
bool VtkScalingActor::visible() const { return _viewer && _actor->GetVisibility();}

void VtkScalingActor::setPosition( const cv::Vec3f& v)
{
    vtkNew<vtkPolyData> pointSet;
    vtkNew<vtkPoints> points;
    points->InsertPoint( 0, &std::vector<double>({v[0], v[1], v[2]})[0]);
    pointSet->SetPoints( points);
    _d2cam->SetInputData( pointSet);
}   // end setPosition

cv::Vec3f VtkScalingActor::position() const
{
    vtkPointSet* pset = _d2cam->GetOutput();
    double x[3];
    pset->GetPoint( 0, x);
    return cv::Vec3f( float(x[0]), float(x[1]), float(x[2]));
}   // end position


void VtkScalingActor::setColour( double r, double g, double b) { _actor->GetProperty()->SetColor( r, g, b);}
void VtkScalingActor::setColour( const double c[3]) { _actor->GetProperty()->SetColor( const_cast<double*>(c));}
const double* VtkScalingActor::colour() const { return _actor->GetProperty()->GetColor();}

void VtkScalingActor::setOpacity( double a) { _actor->GetProperty()->SetOpacity(a);}
double VtkScalingActor::opacity() const { return _actor->GetProperty()->GetOpacity();}


void VtkScalingActor::copyPropertiesFrom( const VtkScalingActor* sa)
{
    setPickable(sa->pickable());
    setFixedScale(sa->fixedScale());
    setScaleFactor(sa->scaleFactor());
    setVisible(sa->visible());
    setPosition(sa->position());
    setColour(sa->colour());
    setOpacity(sa->opacity());
    pokeTransform(sa->_actor->GetMatrix());
}   // end copyPropertiesFrom


bool VtkScalingActor::isProp( const vtkProp* p) const { return _actor == p;}
bool VtkScalingActor::pointedAt( const QPoint& p) const { return _viewer && _viewer->pointedAt( p, _actor);}


void VtkScalingActor::setInViewer( VtkActorViewer* viewer)
{
    if ( _viewer)   // Remove from existing viewer if set
    {
        _viewer->remove(_actor);
        _d2cam->SetRenderer( nullptr);
        _viewer = nullptr;
    }   // end if

    if ( viewer)
    {
        viewer->add(_actor);
        _d2cam->SetRenderer( viewer->getRenderer());
        _viewer = viewer;
    }   // end if
}   // end setInViewer


void VtkScalingActor::pokeTransform( const vtkMatrix4x4* vm) { _actor->PokeMatrix( const_cast<vtkMatrix4x4*>(vm));}


void VtkScalingActor::fixTransform()
{
    cv::Vec3f vpos = position();
    const vtkMatrix4x4* vm = _actor->GetMatrix();
    const RFeatures::Transformer mover( RVTK::toCV(vm));
    mover.transform(vpos);  // In-place
    setPosition(vpos);
}   // end fixTransform

