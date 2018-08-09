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
 * This class encapsulates the functionality needed for the fixed scaling of an actor.
 */

#include "QTools_Export.h"
#include <opencv2/opencv.hpp>
#include <vtkRenderer.h>
#include <vtkDistanceToCamera.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkGlyph3D.h>
#include <vtkActor.h>

namespace QTools {

class QTools_EXPORT VtkScalingActor
{
public:
    explicit VtkScalingActor( vtkPolyDataAlgorithm* src);

    // It is up to the caller to ensure that the renderer that the prop is
    // added to is provided as a parameter to setRenderer. It is possible
    // to assign a different renderer (for distance calculations) than the
    // one the prop is added to.
    const vtkActor* prop() const { return _actor;}  // Get the prop to add to the renderer.
    void setRenderer( vtkRenderer*);                // Set the associated renderer.
    vtkRenderer* renderer() const;                  // Return the associated renderer.

    void setFixedScale( bool);              // False initially.
    bool fixedScale() const;                // Will return true if doing fixed scaling.

    void setPickable( bool);
    bool pickable() const;

    void setScaleFactor( double);
    double scaleFactor() const;

    void setVisible( bool);
    bool visible() const;

    void setPosition( const cv::Vec3f&);
    cv::Vec3f position() const;

    void setColour( double r, double g, double b);
    void setColour( const double[3]);
    const double* colour() const;

    void setOpacity( double);
    double opacity() const;

    void pokeTransform( const vtkMatrix4x4*);   // Directly adjust the actor's transform.
    void fixTransform();                        // Actor transform is Identity on return.

    // Copy properties from the provided actor to this one (including renderer).
    void copyProperties( const VtkScalingActor&);

private:
    vtkNew<vtkGlyph3D> _glyph;
    vtkNew<vtkDistanceToCamera> _d2cam;
    vtkNew<vtkActor> _actor;

    VtkScalingActor( const VtkScalingActor&) = delete;
    void operator=( const VtkScalingActor&) = delete;
};  // end class

}   // end namespace

#endif
