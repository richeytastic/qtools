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

#ifndef QTOOLS_VTK_SCALING_ACTOR_H
#define QTOOLS_VTK_SCALING_ACTOR_H

/**
 * This class exists because scaling actors cannot be added to more than one renderer
 * since their size is dependent upon the camera present.
 */

#include "VtkActorViewer.h"
#include <vtkDistanceToCamera.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkGlyph3D.h>

namespace QTools {

class QTools_EXPORT VtkScalingActor
{
public:
    // Initially, the scale of the actor is fixed with respect to the camera (viewer).
    // Call function setFixedScale to turn fixed scaling on and off.
    VtkScalingActor( vtkPolyDataAlgorithm* src, double x=0.0, double y=0.0, double z=0.0);
    VtkScalingActor( vtkPolyDataAlgorithm* src, const cv::Vec3f& pos);

    void setPickable( bool);
    bool pickable() const;

    void setFixedScale( bool);
    bool fixedScale() const;

    void setScaleFactor( double);
    double scaleFactor() const;

    void setVisible( bool);
    bool visible() const;   // Only visible if setVisible(true) and setInViewer.

    void setPosition( const cv::Vec3f&);
    cv::Vec3f position() const;

    void setColour( double r, double g, double b);
    void setColour( const double[3]);
    const double* colour() const;

    void setOpacity( double);
    double opacity() const;

    // Copy the above properties in to this object from the given object.
    // Does NOT assign this object to a viewer!
    void copyPropertiesFrom( const VtkScalingActor*);

    bool pointedAt( const QPoint&) const;
    bool isProp( const vtkProp*) const;
    const vtkProp* prop() const { return _actor;}

    // This actor can only be set in a single viewer since its scale depends
    // upon the position of the camera in the viewer it's been added to.
    // Initially, the actor is not set in a viewer. Move this actor between
    // viewers by calling setInViewer with a different viewer parameter.
    // Call with nullptr (default) to remove from the viewer.
    void setInViewer( VtkActorViewer* v=nullptr);
    const VtkActorViewer* viewer() const { return _viewer;}

    void pokeTransform( const vtkMatrix4x4*);   // Directly adjust the actor's transform.
    void fixTransform();                        // Actor transform is Identity on return.

private:
    vtkNew<vtkGlyph3D> _glyph;
    vtkNew<vtkDistanceToCamera> _d2cam;
    vtkNew<vtkActor> _actor;
    VtkActorViewer *_viewer;

    void init( vtkPolyDataAlgorithm*, const cv::Vec3f&);
    VtkScalingActor( const VtkScalingActor&) = delete;
    void operator=( const VtkScalingActor&) = delete;
};  // end class

}   // end namespace

#endif
