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
    virtual ~VtkScalingActor();

    void setPickable( bool);
    bool pickable() const;

    void setFixedScale( bool);
    bool fixedScale() const;

    void setScaleFactor( double);
    double scaleFactor() const;

    void setVisible( bool);
    bool visible() const;

    void setPosition( const cv::Vec3f&);
    cv::Vec3f position() const;

    void setColour( double r, double g, double b);
    const double* colour() const;

    void setOpacity( double);
    double opacity() const;

    bool isProp( const vtkProp*) const;
    bool pointedAt( const QPoint&) const;

    void addToViewer( QTools::VtkActorViewer*);
    void removeFromViewer();

    void pokeTransform( const vtkMatrix4x4*);   // Directly adjust the actor's transform.
    void fixTransform();                        // Actor transform is Identity on return.

private:
    vtkSmartPointer<vtkGlyph3D> _glyph;
    vtkSmartPointer<vtkDistanceToCamera> _d2cam;
    vtkSmartPointer<vtkActor> _actor;
    QTools::VtkActorViewer *_viewer;

    void init( vtkPolyDataAlgorithm*, const cv::Vec3f&);
    VtkScalingActor( const VtkScalingActor&) = delete;
    void operator=( const VtkScalingActor&) = delete;
};  // end class

}   // end namespace

#endif
