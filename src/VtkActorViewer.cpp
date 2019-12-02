/************************************************************************
 * Copyright (C) 2019 Richard Palmer
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

#include <VtkActorViewer.h>
#include <r3dvis/RendererPicker.h>
#include <cassert>
#include <iostream>
#include <vtkLight.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkFollower.h>
using QTools::VtkActorViewer;
using QTools::VVI;
using QTools::VMH;
using QTools::KeyPressHandler;
using r3d::CameraParams;
using r3d::Vec3f;


VtkActorViewer::VtkActorViewer( QWidget *parent)
    : QVTKOpenGLWidget( parent), _autoUpdateRender(false)
{
    //setUpdateBehavior( QOpenGLWidget::NoPartialUpdate);
    //requireRenderWindowInitialization();

    SetRenderWindow( _rwin);

    _ren->SetBackground( 0., 0., 0.);
    _ren->SetTwoSidedLighting( true);   // Don't light occluded sides
    _ren->SetAutomaticLightCreation( false);

    _rwin->SetStereoCapableWindow(1);
    _rwin->SetStereoTypeToRedBlue();

    _rwin->SetPointSmoothing( true);
    _rwin->AddRenderer( _ren);

    _iman = new VtkViewerInteractorManager(this);
    setEnableHiDPI(true);
}	// end ctor


VtkActorViewer::~VtkActorViewer() { delete _iman;}


void VtkActorViewer::setInteractor( vtkInteractorStyle* iStyle)
{
    //iStyle->SetDefaultRenderer( _ren);
    _rwin->GetInteractor()->SetInteractorStyle( iStyle);
}   // end setInteractor


void VtkActorViewer::updateRender()
{
    _ren->ResetCameraClippingRange();
    _rwin->Render();
}   // end updateRender


void VtkActorViewer::setSize( size_t w, size_t h)
{
    this->resize( (int)w, (int)h);
    this->setMinimumSize( (int)w, (int)h);
    this->setMaximumSize( (int)w, (int)h);
}   // end setSize


cv::Mat_<float> VtkActorViewer::getRawZBuffer() const { return r3dvis::extractZBuffer( _rwin);}


cv::Mat_<cv::Vec3b> VtkActorViewer::getColourImg() const { return r3dvis::extractImage( _rwin);}


void VtkActorViewer::add( vtkProp* prop)
{
    _ren->AddViewProp( prop);
    if ( prop->IsA("vtkFollower"))
        vtkFollower::SafeDownCast(prop)->SetCamera( _ren->GetActiveCamera());
    if ( _autoUpdateRender)
        updateRender();
}   // end add


void VtkActorViewer::remove( vtkProp* prop)
{
    _ren->RemoveViewProp( prop);
    if ( _autoUpdateRender)
        updateRender();
}   // end remove


void VtkActorViewer::clear()
{
    _ren->RemoveAllViewProps();
    if ( _autoUpdateRender)
        updateRender();
}   // end clear


// public
CameraParams VtkActorViewer::camera() const
{
    vtkCamera* cam = _ren->GetActiveCamera();
    const double *arr = cam->GetPosition();
    const Vec3f pos = Vec3f( (float)arr[0], (float)arr[1], (float)arr[2]);

    arr = cam->GetFocalPoint();
    const Vec3f foc = Vec3f( (float)arr[0], (float)arr[1], (float)arr[2]);

    arr = cam->GetViewUp();
    const Vec3f upv = Vec3f( (float)arr[0], (float)arr[1], (float)arr[2]);

    float fov = cam->GetViewAngle();
    return CameraParams( pos, foc, upv, fov);
}   // end camera


void VtkActorViewer::setCamera( const CameraParams& cp)
{
    vtkCamera* cam = _ren->GetActiveCamera();
    cam->SetFocalPoint( cp.focus()[0], cp.focus()[1], cp.focus()[2]);
    cam->SetPosition( cp.pos()[0], cp.pos()[1], cp.pos()[2]);
    cam->SetViewUp( cp.up()[0], cp.up()[1], cp.up()[2]);
    cam->SetViewAngle( cp.fov());
    //_ren->ResetCameraClippingRange();
    if ( _autoUpdateRender)
        updateRender();
}   // end setCamera


void VtkActorViewer::refreshClippingPlanes()
{
    vtkCamera* vcamera = getRenderer()->GetActiveCamera();
    double cmin, cmax;  // Get the min and max clipping ranges
    vcamera->GetClippingRange( cmin, cmax);
    const CameraParams cp = camera();
    /*
    std::cerr << "Clipping plane range min --> max: " << cmin << " --> " << cmax << std::endl;
    std::cerr << "  Camera position:  " << cp.pos << std::endl;
    std::cerr << "  Camera focus:     " << cp.focus << std::endl;
    */
    const double pfdelta = cp.distance();
    //std::cerr << "  Position - Focus: " << pfdelta << std::endl;
    // If the distance between the camera position and the focus is less than 2% the
    // distance to the near clipping plane, then make the near clipping plane closer.
    cmin = 0.01 * cmax;
    const double ctol =  2*cmin > pfdelta ? 0.00001 : 0.01;
    getRenderer()->SetNearClippingPlaneTolerance(ctol);
    getRenderer()->ResetCameraClippingRange();
    updateRender();
}   // end refreshClippingPlanes


void VtkActorViewer::setStereoRendering( bool on)
{
    _rwin->SetStereoRender( on);
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
        //cam->SetParallelScale( 10.0); // For parallel projection
    }   // end if
    else
    {
        cam->ParallelProjectionOff();
        //cam->SetViewAngle( _resetCamera.fov);   // For perspective
    }   // end else

    //_ren->ResetCameraClippingRange();
    if ( _autoUpdateRender)
        updateRender();
}	// end setOrthogonal


// public
void VtkActorViewer::setLights( const std::vector<r3dvis::Light>& lights)
{
    r3dvis::resetLights( _ren, lights);
    if ( _autoUpdateRender)
        updateRender();
}   // end setLights


// public
bool VtkActorViewer::pointedAt( const cv::Point& p, const vtkProp* prop) const
{
    if ( prop)
    {
        r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
        return prop == picker.pickActor( p);
    }   // end if
    return false;
}   // end pointedAt


// public
bool VtkActorViewer::pointedAt( const QPoint& p, const vtkProp* prop) const
{
    return pointedAt( cv::Point(p.x(), p.y()), prop);
}   // end pointedAt


// public
vtkActor* VtkActorViewer::pickActor( const cv::Point& p, const std::vector<vtkActor*>& pactors) const
{
    r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
    return picker.pickActor( p, pactors);
}   // end pickActor


// public
vtkActor* VtkActorViewer::pickActor( const QPoint& p, const std::vector<vtkActor*>& pactors) const
{
    return pickActor( cv::Point(p.x(), p.y()), pactors);
}   // end pickActor


// public
vtkActor* VtkActorViewer::pickActor( const cv::Point& p) const
{
    r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
    return picker.pickActor( p);
}   // end pickActor


// public
vtkActor* VtkActorViewer::pickActor( const QPoint& p) const
{
    return pickActor( cv::Point(p.x(), p.y()));
}   // end pickActor


// public
int VtkActorViewer::pickCell(const cv::Point &p) const
{
    r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
    return picker.pickCell( p);
}   // end pickCell


// public
int VtkActorViewer::pickCell(const QPoint &p) const
{
    return pickCell( cv::Point(p.x(), p.y()));
}   // end pickCell


// public
int VtkActorViewer::pickActorCells( const std::vector<cv::Point>& points, vtkActor* actor, std::vector<int>& cellIds) const
{
    r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
    return picker.pickActorCells( points, actor, cellIds);
}   // end pickActorCells


// public
int VtkActorViewer::pickActorCells( const std::vector<QPoint>& points, vtkActor* actor, std::vector<int>& cellIds) const
{
    std::vector<cv::Point> pts;
    std::for_each( std::begin(points), std::end(points), [&](QPoint p){ pts.push_back(cv::Point(p.x(), p.y()));});
    return pickActorCells( pts, actor, cellIds);
}   // end pickActorCells


// public
Vec3f VtkActorViewer::pickWorldPosition( const cv::Point& p) const
{
    r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
    return picker.pickWorldPosition( p);
}   // end pickWorldPosition


// public
Vec3f VtkActorViewer::pickWorldPosition( const QPoint& p) const
{
    return pickWorldPosition( cv::Point(p.x(), p.y()));
}   // end pickWorldPosition


// public
Vec3f VtkActorViewer::pickWorldPosition( const cv::Point2f& p) const
{
    const cv::Point np( (int)cvRound(p.x * (getWidth()-1)), (int)cvRound(p.y * (getHeight()-1)));
    return pickWorldPosition( np);
}   // end pickWorldPosition


// public
cv::Point VtkActorViewer::projectToDisplay( const Vec3f& v) const
{
    r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
    return picker.projectToImagePlane( v);
}   // end projectToDisplay


// public
cv::Point2f VtkActorViewer::projectToDisplayProportion( const Vec3f& v) const
{
    const cv::Point p = projectToDisplay(v);
    return cv::Point2f( float(p.x) / (getWidth()-1), float(p.y) / (getHeight()-1));
}   // end projectToDisplayProportion


// public
void VtkActorViewer::attachKeyPressHandler( KeyPressHandler* kph) { _keyPressHandlers.insert( kph);}
void VtkActorViewer::detachKeyPressHandler( KeyPressHandler* kph) { _keyPressHandlers.erase( kph);}


// public
bool VtkActorViewer::isAttached( VMH* v) const { return _iman->isAttached(v);}
bool VtkActorViewer::isAttached( VVI* v) const { return _iman->isAttached(v);}


// public
bool VtkActorViewer::attach( VVI* vvi)
{
    if (isAttached(vvi))
        return false;
    _iman->addInteractor(vvi);
    return true;
}   // end attach

// public
bool VtkActorViewer::attach( VMH* vmh)
{
    if (isAttached(vmh))
        return false;
    _iman->addMouseHandler(vmh);
    return true;
}   // end attach

// public
bool VtkActorViewer::detach( VVI* vvi)
{
    if (!isAttached(vvi))
        return false;
    _iman->removeInteractor(vvi);
    return true;
}   // end detach

// public
bool VtkActorViewer::detach( VMH* vmh)
{
    if (!isAttached(vmh))
        return false;
    _iman->removeMouseHandler(vmh);
    return true;
}   // end detach


// protected
void VtkActorViewer::keyPressEvent( QKeyEvent* event)
{
    //std::cerr << "QTools::VtkActorViewer:: KEY PRESSED" << std::endl;
    bool accepted = false;
    for ( KeyPressHandler* kph : _keyPressHandlers)
        accepted = accepted || kph->handleKeyPress(event);
    if ( !accepted)
        QVTKOpenGLWidget::keyPressEvent( event);
}   // end keyPressEvent


// protected
void VtkActorViewer::keyReleaseEvent( QKeyEvent* event)
{
    bool accepted = false;
    for ( KeyPressHandler* kph : _keyPressHandlers)
        accepted = accepted || kph->handleKeyRelease(event);
    if ( !accepted)
        QVTKOpenGLWidget::keyReleaseEvent( event);
}   // end keyReleaseEvent
