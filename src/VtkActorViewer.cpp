/************************************************************************
 * Copyright (C) 2021 Richard Palmer
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
    : QVTKOpenGLNativeWidget( parent), _autoUpdateRender(false)
{
    _rwin->SetMultiSamples(0);
    _rwin->SetPointSmoothing( false);
    _rwin->SetStereoCapableWindow(1);
    _rwin->SetStereoTypeToRedBlue();
    _rwin->AddRenderer( _ren);

    _ren->SetBackground( 0.0, 0.0, 0.0);
    _ren->SetTwoSidedLighting( true);
    _ren->SetAutomaticLightCreation( false);

    setRenderWindow( _rwin);

    _iman = new VtkViewerInteractorManager(this);
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
    this->setMinimumSize( QSize((int)w, (int)h));
    this->setMaximumSize( QSize((int)w, (int)h));
}   // end setSize


cv::Mat_<float> VtkActorViewer::getRawZBuffer()
{
    updateRender();
    return r3dvis::extractZ( _rwin);
}   // end getRawZBuffer();


cv::Mat_<cv::Vec3b> VtkActorViewer::getColourImg()
{
    updateRender();
    return r3dvis::extractBGR( _rwin);
}   // end getColourImg


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


CameraParams VtkActorViewer::camera() const
{
    vtkCamera* cam = _ren->GetActiveCamera();
    const double *arr = cam->GetPosition();
    const Vec3f pos = Vec3f( (float)arr[0], (float)arr[1], (float)arr[2]);

    arr = cam->GetFocalPoint();
    const Vec3f foc = Vec3f( (float)arr[0], (float)arr[1], (float)arr[2]);

    arr = cam->GetViewUp();
    const Vec3f upv = Vec3f( (float)arr[0], (float)arr[1], (float)arr[2]);

    const double fov = cam->GetViewAngle();
    return CameraParams( pos, foc, upv, float(fov));
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


void VtkActorViewer::setLights( const std::vector<r3dvis::Light>& lights)
{
    r3dvis::resetLights( _ren, lights);
    if ( _autoUpdateRender)
        updateRender();
}   // end setLights


bool VtkActorViewer::pointedAt( const cv::Point& p, const vtkProp* prop) const
{
    if ( p.x < 0 || p.y < 0 || p.x >= _ren->GetSize()[0] || p.y >= _ren->GetSize()[1] || !prop)
        return false;
    r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
    return prop == picker.pickActor( p);
}   // end pointedAt


bool VtkActorViewer::pointedAt( const QPoint& p, const vtkProp* prop) const
{
    return pointedAt( cv::Point(p.x(), p.y()), prop);
}   // end pointedAt


const vtkActor* VtkActorViewer::pickActor( const cv::Point& p, const std::vector<const vtkProp*>& pactors) const
{
    if ( p.x < 0 || p.y < 0 || p.x >= _ren->GetSize()[0] || p.y >= _ren->GetSize()[1])
        return nullptr;
    r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
    return picker.pickActor( p, pactors);
}   // end pickActor


const vtkActor* VtkActorViewer::pickActor( const QPoint& p, const std::vector<const vtkProp*>& pactors) const
{
    return pickActor( cv::Point(p.x(), p.y()), pactors);
}   // end pickActor


const vtkActor* VtkActorViewer::pickActor( const cv::Point& p) const
{
    if ( p.x < 0 || p.y < 0 || p.x >= _ren->GetSize()[0] || p.y >= _ren->GetSize()[1])
        return nullptr;
    r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
    return picker.pickActor( p);
}   // end pickActor


const vtkActor* VtkActorViewer::pickActor( const QPoint& p) const
{
    return pickActor( cv::Point(p.x(), p.y()));
}   // end pickActor


int VtkActorViewer::pickCell( const cv::Point &p) const
{
    if ( p.x < 0 || p.y < 0 || p.x >= _ren->GetSize()[0] || p.y >= _ren->GetSize()[1])
        return -1;
    r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
    return picker.pickCell( p);
}   // end pickCell


int VtkActorViewer::pickCell(const QPoint &p) const
{
    return pickCell( cv::Point(p.x(), p.y()));
}   // end pickCell


int VtkActorViewer::pickActorCells( const std::vector<cv::Point>& points,
                                    const vtkProp* actor, std::vector<int>& cellIds) const
{
    r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
    return picker.pickActorCells( points, actor, cellIds);
}   // end pickActorCells


int VtkActorViewer::pickActorCells( const std::vector<QPoint>& points,
                                    const vtkProp* actor, std::vector<int>& cellIds) const
{
    std::vector<cv::Point> pts;
    std::for_each( std::begin(points), std::end(points), [&](QPoint p){ pts.push_back(cv::Point(p.x(), p.y()));});
    return pickActorCells( pts, actor, cellIds);
}   // end pickActorCells


Vec3f VtkActorViewer::pickPosition( const cv::Point& p) const
{
    if ( p.x < 0 || p.y < 0 || p.x >= _ren->GetSize()[0] || p.y >= _ren->GetSize()[1])
        return Vec3f::Zero();
    r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
    return picker.pickPosition( p);
}   // end pickPosition


Vec3f VtkActorViewer::pickPosition( const QPoint& p) const
{
    return pickPosition( cv::Point(p.x(), p.y()));
}   // end pickPosition


Vec3f VtkActorViewer::pickPosition( const cv::Point2f& p) const
{
    const int w = _ren->GetSize()[0];
    const int h = _ren->GetSize()[1];
    const cv::Point np( std::min( int(p.x * w), w-1),
                        std::min( int(p.y * h), h-1));
    return pickPosition( np);
}   // end pickPosition


bool VtkActorViewer::pickPosition( const vtkProp *actor, const cv::Point &p, Vec3f &v) const
{
    if ( p.x < 0 || p.y < 0 || p.x >= _ren->GetSize()[0] || p.y >= _ren->GetSize()[1])
        return false;
    r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
    return picker.pickPosition( actor, p, v);
}   // end pickPosition


bool VtkActorViewer::pickPosition( const vtkProp *actor, const QPoint &p, Vec3f &v) const
{
    return pickPosition( actor, cv::Point(p.x(), p.y()), v);
}   // end pickPosition


bool VtkActorViewer::pickPosition( const vtkProp *actor, const cv::Point2f &p, Vec3f &v) const
{
    const int w = _ren->GetSize()[0];
    const int h = _ren->GetSize()[1];
    const cv::Point np( std::min( int(p.x * w), w-1),
                        std::min( int(p.y * h), h-1));
    return pickPosition( actor, np, v);
}   // end pickPosition


cv::Point VtkActorViewer::projectToDisplay( const Vec3f& v) const
{
    r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
    return picker.projectToImagePlane( v);
}   // end projectToDisplay


QPoint VtkActorViewer::projectToDisplayPoint( const Vec3f& v) const
{
    r3dvis::RendererPicker picker( _ren, r3dvis::RendererPicker::TOP_LEFT);
    cv::Point p = picker.projectToImagePlane( v);
    return QPoint( p.x, p.y);
}   // end projectToDisplayPoint


cv::Point2f VtkActorViewer::projectToDisplayProportion( const Vec3f& v) const
{
    const cv::Point p = projectToDisplay(v);
    return cv::Point2f( (float(p.x) + 0.5f) / getWidth(), (float(p.y) + 0.5f) / getHeight());
}   // end projectToDisplayProportion


void VtkActorViewer::attachKeyPressHandler( KeyPressHandler* kph) { _keyPressHandlers.insert( kph);}
void VtkActorViewer::detachKeyPressHandler( KeyPressHandler* kph) { _keyPressHandlers.erase( kph);}


bool VtkActorViewer::isAttached( VMH* v) const { return _iman->isAttached(v);}
bool VtkActorViewer::isAttached( VVI* v) const { return _iman->isAttached(v);}


bool VtkActorViewer::attach( VVI* vvi)
{
    if (isAttached(vvi))
        return false;
    _iman->addInteractor(vvi);
    return true;
}   // end attach

bool VtkActorViewer::attach( VMH* vmh)
{
    if (isAttached(vmh))
        return false;
    _iman->addMouseHandler(vmh);
    return true;
}   // end attach

bool VtkActorViewer::detach( VVI* vvi)
{
    if (!isAttached(vvi))
        return false;
    _iman->removeInteractor(vvi);
    return true;
}   // end detach

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
        QVTKOpenGLNativeWidget::keyPressEvent( event);
}   // end keyPressEvent


// protected
void VtkActorViewer::keyReleaseEvent( QKeyEvent* event)
{
    bool accepted = false;
    for ( KeyPressHandler* kph : _keyPressHandlers)
        accepted = accepted || kph->handleKeyRelease(event);
    if ( !accepted)
        QVTKOpenGLNativeWidget::keyReleaseEvent( event);
}   // end keyReleaseEvent
