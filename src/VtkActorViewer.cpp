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

#include "VtkActorViewer.h"
#include <VtkTools.h>       // RVTK
#include <RendererPicker.h> // RVTK
#include <FeatureUtils.h>   // RFeatures
#include <cassert>
#include <iostream>
#include <vtkLight.h>
#include <vtkRendererCollection.h>
#include <vtkOpenGLRenderer.h>
//#include <vtkGraphicsFactory.h>
using QTools::VtkActorViewer;
using RFeatures::CameraParams;


VtkActorViewer::VtkActorViewer( QWidget *parent, bool offscreen)
    : QVTKWidget( parent), _autoUpdateRender(false),
     _ren( vtkOpenGLRenderer::New()), _rwindow( NULL), _rpicker(NULL), _resetCamera()
{
    /*
    // Check if vtkGraphicsFactory is really needed...
    vtkSmartPointer<vtkGraphicsFactory> gf;
    if ( offscreen)
    {
        //gf = vtkSmartPointer<vtkGraphicsFactory>::New();
        //gf->SetOffScreenOnlyMode(true);
        //gf->SetUseMesaClasses(true);
    }   // end if
    */
        
    //QWidget::setWindowFlags(Qt::Window);
    _ren->SetBackground( 0., 0., 0.);
    _ren->SetTwoSidedLighting( true);   // Don't light occluded sides
    _ren->SetAutomaticLightCreation( false);

    _rwindow = this->GetRenderWindow();
    _rwindow->SetPointSmoothing( false);
    _rwindow->SetOffScreenRendering( offscreen);
    _rwindow->AddRenderer( _ren);

    _rpicker = new RVTK::RendererPicker( _ren, RVTK::RendererPicker::TOP_LEFT);
}	// end ctor


VtkActorViewer::~VtkActorViewer()
{
    _ren->Delete();
    delete _rpicker;
}   // end dtor


// public
void VtkActorViewer::setInteractor( vtkSmartPointer<vtkInteractorStyle> intStyle)
{
    _rwindow->GetInteractor()->SetInteractorStyle( intStyle);
    intStyle->GetInteractor()->SetRenderWindow( _rwindow);
}   // end setInteractor


// public
void VtkActorViewer::updateRender()
{
    _ren->ResetCameraClippingRange();
    _rwindow->Render();
}   // end updateRender


// public
void VtkActorViewer::setSize( int w, int h)
{
    this->resize(w,h);
    this->setMinimumSize(w,h);
    this->setMaximumSize(w,h);
}   // end setSize


// public
int VtkActorViewer::getWidth() const
{
    return _ren->GetSize()[0];
}   // end getWidth


// public
int VtkActorViewer::getHeight() const
{
    return _ren->GetSize()[1];
}   // end getHeight


// public
cv::Size VtkActorViewer::getSize() const
{
    const int* wsz = _ren->GetSize();
    return cv::Size( wsz[0], wsz[1]);
}   // end getSize


// public
cv::Mat_<float> VtkActorViewer::getRawZBuffer() const
{
    return RVTK::extractZBuffer( _rwindow);
}   // end getRawZBuffer


// public
cv::Mat_<cv::Vec3b> VtkActorViewer::getColourImg() const
{
    return RVTK::extractImage( _rwindow);
}   // end getColourImg


// public
void VtkActorViewer::addActor( vtkActor* actor)
{
    _ren->AddActor( actor);
    if ( _autoUpdateRender)
        updateRender();
}   // end addActor


// public
void VtkActorViewer::removeActor( vtkActor* obj)
{
    _ren->RemoveActor( obj);
    if ( _autoUpdateRender)
        updateRender();
}   // end removeActor


// public
void VtkActorViewer::clear()
{
    _ren->RemoveAllViewProps();
    if ( _autoUpdateRender)
        updateRender();
}   // end clear


// public
void VtkActorViewer::setResetCamera( const CameraParams& cp)
{
    _resetCamera = cp;
    resetCamera();
}   // end setResetCamera


// public
void VtkActorViewer::resetCamera()
{
    setCamera( _resetCamera);
}   // end resetCamera


// public
void VtkActorViewer::getCamera( CameraParams& cp) const
{
    vtkCamera* cam = _ren->GetActiveCamera();

    const double *arr = cam->GetPosition();
    cp.pos = cv::Vec3f( (float)arr[0], (float)arr[1], (float)arr[2]);

    arr = cam->GetFocalPoint();
    cp.focus = cv::Vec3f( (float)arr[0], (float)arr[1], (float)arr[2]);

    arr = cam->GetViewUp();
    cp.up = cv::Vec3f( (float)arr[0], (float)arr[1], (float)arr[2]);

    cp.fov = cam->GetViewAngle();
}   // end getCamera


// public
void VtkActorViewer::setCamera( const CameraParams& cp)
{
    vtkCamera* cam = _ren->GetActiveCamera();
    cam->SetFocalPoint( cp.focus[0], cp.focus[1], cp.focus[2]);
    cam->SetPosition( cp.pos[0], cp.pos[1], cp.pos[2]);
    cam->ComputeViewPlaneNormal();
    cam->SetViewUp( cp.up[0], cp.up[1], cp.up[2]);
    cam->SetViewAngle( cp.fov);
    _ren->ResetCameraClippingRange();
    if ( _autoUpdateRender)
        updateRender();
}   // end setCamera


// public
void VtkActorViewer::setBackgroundWhite( bool on)
{
    const double c = on ? 255 : 0;
    _ren->SetBackground( c, c, c);
    if ( _autoUpdateRender)
        updateRender();
}	// end setBackgroundWhite


// public
void VtkActorViewer::setStereoRendering( bool on)
{
    _rwindow->SetStereoRender( on);
    if ( _autoUpdateRender)
        updateRender();
}	// end setStereoRendering


// public
void VtkActorViewer::setOrthogonal( bool on)
{
    vtkCamera* cam = _ren->GetActiveCamera();
    if ( on)
    {
        cam->ParallelProjectionOn();
        cam->SetParallelScale( 10.0); // For parallel projection
    }   // end if
    else
    {
        cam->ParallelProjectionOff();
        cam->SetViewAngle( _resetCamera.fov);   // For perspective
    }   // end else

    _ren->ResetCameraClippingRange();
    if ( _autoUpdateRender)
        updateRender();
}	// end setOrthogonal


// public
void VtkActorViewer::setSceneLights( const std::vector<cv::Vec3f>& lpos, const std::vector<cv::Vec3f>& lfoc)
{
    const int numLights = (int)lpos.size();
    assert( numLights == lfoc.size());

    _ren->RemoveAllLights();
    for ( int i = 0; i < numLights; ++i)
    {
        vtkSmartPointer<vtkLight> light = vtkSmartPointer<vtkLight>::New();
        light->SetLightTypeToSceneLight();
        light->SetColor(1,1,1);
        light->SetAmbientColor(1,1,1);
        light->SetIntensity( 1);
        light->SetPosition( lpos[i][0], lpos[i][1], lpos[i][2]);
        light->SetFocalPoint( lfoc[i][0], lfoc[i][1], lfoc[i][2]);
        _ren->AddLight( light);
    }   // end for

    if ( _autoUpdateRender)
        updateRender();
}   // end setSceneLights


// public
void VtkActorViewer::setHeadlight()
{
    _ren->RemoveAllLights();
    vtkSmartPointer<vtkLight> hlight = vtkSmartPointer<vtkLight>::New();
    hlight->SetLightTypeToHeadlight();
    _ren->AddLight( hlight);
    _ren->SetLightFollowCamera(true);
    hlight->SetSwitch( true);

    if ( _autoUpdateRender)
        updateRender();
}   // end setHeadlight


// public
vtkActor* VtkActorViewer::pickActor( const cv::Point& p, const std::vector<vtkActor*>& pactors) const
{
    return _rpicker->pickActor( p, pactors);
}   // end pickActor


// public
vtkSmartPointer<vtkActor> VtkActorViewer::pickActor( const cv::Point& p,
                                                     const std::vector<vtkSmartPointer<vtkActor> >& pactors) const
{
    return _rpicker->pickActor( p, pactors);
}   // end pickActor


// public
vtkActor* VtkActorViewer::pickActor( const cv::Point& p) const
{
    return _rpicker->pickActor( p);
}   // end pickActor


// public
int VtkActorViewer::pickCell(const cv::Point &p) const
{
    return _rpicker->pickCell( p);
}   // end pickCell


// public
int VtkActorViewer::pickActorCells( const std::vector<cv::Point>& points, vtkActor* actor, std::vector<int>& cellIds) const
{
    return _rpicker->pickActorCells( points, actor, cellIds);
}   // end pickActorCells


// public
int VtkActorViewer::pickActorCells( const cv::Mat& inmask, vtkActor* actor, std::vector<int>& cellIds) const
{
    if ( actor == NULL)
        return 0;

    assert( inmask.rows == getHeight() && inmask.cols == getWidth());
    assert( inmask.channels() == 1);

    cv::Mat_<byte> mask;
    inmask.convertTo( mask, CV_8U);
    std::vector<cv::Point> pts;
    RFeatures::nonZeroToPoints( mask, pts);
    return pickActorCells( pts, actor, cellIds);
}   // end pickActorCells


// public
cv::Vec3f VtkActorViewer::pickWorldPosition( const cv::Point& p) const
{
    return _rpicker->pickWorldPosition( p);
}   // end pickWorldPosition


// public
cv::Vec3f VtkActorViewer::pickWorldPosition( const cv::Point2f& p) const
{
    const cv::Point np( (int)cvRound(p.x * (getWidth()-1)), (int)cvRound(p.y * (getHeight()-1)));
    return pickWorldPosition( np);
}   // end pickWorldPosition


// public
cv::Vec3f VtkActorViewer::pickNormal( const cv::Point& p) const
{
    return _rpicker->pickNormal( p);
}   // end pickNormal


// public
cv::Point VtkActorViewer::projectToDisplay( const cv::Vec3f& v) const
{
    return _rpicker->projectToImagePlane( v);
}   // end projectToDisplay


// public
cv::Point2f VtkActorViewer::projectToDisplayProportion( const cv::Vec3f& v) const
{
    const cv::Point p = projectToDisplay(v);
    return cv::Point2f( float(p.x) / (getWidth()-1), float(p.y) / (getHeight()-1));
}   // end projectToDisplayProportion
