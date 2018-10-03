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

#include <VtkActorViewer.h>
#include <RendererPicker.h> // RVTK
#include <FeatureUtils.h>   // RFeatures
#include <cassert>
#include <iostream>
#include <vtkLight.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkFollower.h>
using QTools::VtkActorViewer;
using QTools::VVI;
using QTools::KeyPressHandler;
using RFeatures::CameraParams;

VtkActorViewer::VtkActorViewer( QWidget *parent)
    //: QVTKOpenGLWidget( parent), _autoUpdateRender(false), _rpicker(nullptr), _resetCamera()
    : QVTKWidget( parent), _autoUpdateRender(false), _rpicker(nullptr), _resetCamera()
{
    _ren = vtkRenderer::New();
    _ren->SetBackground( 0., 0., 0.);
    _ren->SetTwoSidedLighting( true);   // Don't light occluded sides
    _ren->SetAutomaticLightCreation( false);
    /*
    _rwin = vtkGenericOpenGLRenderWindow::New();
    SetRenderWindow( _rwin);
    */
    _rwin = this->GetRenderWindow();  // Only when inheriting from QVTKWidget
    assert(_rwin);
    _rwin->SetStereoCapableWindow(1);
    _rwin->SetStereoTypeToRedBlue();

    //_rwin->SetPointSmoothing( false);
    _rwin->AddRenderer( _ren);

    _rpicker = new RVTK::RendererPicker( _ren, RVTK::RendererPicker::TOP_LEFT);

    _iman = new VtkViewerInteractorManager(this);
}	// end ctor


// public
VtkActorViewer::~VtkActorViewer()
{
    delete _iman;
    delete _rpicker;
    _ren->Delete();
    //_rwin->Delete();
}   // end dtor


void VtkActorViewer::setInteractor( vtkInteractorStyle* iStyle) { _rwin->GetInteractor()->SetInteractorStyle( iStyle);}


// public
void VtkActorViewer::updateRender()
{
    _ren->ResetCameraClippingRange();
    _rwin->Render();
}   // end updateRender


// public
void VtkActorViewer::setSize( size_t w, size_t h)
{
    this->resize( (int)w, (int)h);
    this->setMinimumSize( (int)w, (int)h);
    this->setMaximumSize( (int)w, (int)h);
}   // end setSize


// public
cv::Mat_<float> VtkActorViewer::getRawZBuffer() const
{
    return RVTK::extractZBuffer( _rwin);
}   // end getRawZBuffer


// public
cv::Mat_<cv::Vec3b> VtkActorViewer::getColourImg() const
{
    return RVTK::extractImage( _rwin);
}   // end getColourImg


// public
void VtkActorViewer::add( vtkProp* prop)
{
    _ren->AddViewProp( prop);
    if ( prop->IsA("vtkFollower"))
        vtkFollower::SafeDownCast(prop)->SetCamera( _ren->GetActiveCamera());
    if ( _autoUpdateRender)
        updateRender();
}   // end add


// public
void VtkActorViewer::remove( vtkProp* prop)
{
    _ren->RemoveViewProp( prop);
    if ( _autoUpdateRender)
        updateRender();
}   // end remove


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
void VtkActorViewer::setLights( const std::vector<RVTK::Light>& lights)
{
    RVTK::resetLights( _ren, lights);
    if ( _autoUpdateRender)
        updateRender();
}   // end setLights


// public
bool VtkActorViewer::pointedAt( const cv::Point& p, const vtkProp* prop) const { return prop && prop == _rpicker->pickActor( p);}
bool VtkActorViewer::pointedAt( const QPoint& p, const vtkProp* prop) const { return pointedAt( cv::Point(p.x(), p.y()), prop);}


// public
vtkActor* VtkActorViewer::pickActor( const cv::Point& p, const std::vector<vtkActor*>& pactors) const
{ return _rpicker->pickActor( p, pactors);}
vtkActor* VtkActorViewer::pickActor( const QPoint& p, const std::vector<vtkActor*>& pactors) const
{ return pickActor( cv::Point(p.x(), p.y()), pactors);}


// public
vtkActor* VtkActorViewer::pickActor( const cv::Point& p) const { return _rpicker->pickActor( p);}
vtkActor* VtkActorViewer::pickActor( const QPoint& p) const { return pickActor( cv::Point(p.x(), p.y()));}


// public
int VtkActorViewer::pickCell(const cv::Point &p) const { return _rpicker->pickCell( p);}
int VtkActorViewer::pickCell(const QPoint &p) const { return pickCell( cv::Point(p.x(), p.y()));}


// public
int VtkActorViewer::pickActorCells( const std::vector<cv::Point>& points, vtkActor* actor, std::vector<int>& cellIds) const
{ return _rpicker->pickActorCells( points, actor, cellIds);}

int VtkActorViewer::pickActorCells( const std::vector<QPoint>& points, vtkActor* actor, std::vector<int>& cellIds) const
{
    std::vector<cv::Point> pts;
    std::for_each( std::begin(points), std::end(points), [&](QPoint p){ pts.push_back(cv::Point(p.x(), p.y()));});
    return pickActorCells( pts, actor, cellIds);
}   // end pickActorCells


// public
int VtkActorViewer::pickActorCells( const cv::Mat& inmask, vtkActor* actor, std::vector<int>& cellIds) const
{
    if ( actor == nullptr)
        return 0;

    assert( inmask.rows == (int)getHeight() && inmask.cols == (int)getWidth());
    assert( inmask.channels() == 1);

    cv::Mat_<byte> mask;
    inmask.convertTo( mask, CV_8U);
    std::vector<cv::Point> pts;
    RFeatures::nonZeroToPoints( mask, pts);
    return pickActorCells( pts, actor, cellIds);
}   // end pickActorCells


// public
cv::Vec3f VtkActorViewer::pickWorldPosition( const cv::Point& p) const { return _rpicker->pickWorldPosition( p);}
cv::Vec3f VtkActorViewer::pickWorldPosition( const QPoint& p) const { return pickWorldPosition( cv::Point(p.x(), p.y()));}


// public
cv::Vec3f VtkActorViewer::pickWorldPosition( const cv::Point2f& p) const
{
    const cv::Point np( (int)cvRound(p.x * (getWidth()-1)), (int)cvRound(p.y * (getHeight()-1)));
    return pickWorldPosition( np);
}   // end pickWorldPosition


// public
cv::Vec3f VtkActorViewer::pickNormal( const cv::Point& p) const { return _rpicker->pickNormal( p);}
cv::Vec3f VtkActorViewer::pickNormal( const QPoint& p) const { return pickNormal( cv::Point(p.x(), p.y()));}


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


// public
void VtkActorViewer::attachKeyPressHandler( KeyPressHandler* kph) { _keyPressHandlers.insert( kph);}
void VtkActorViewer::detachKeyPressHandler( KeyPressHandler* kph) { _keyPressHandlers.erase( kph);}


// public
bool VtkActorViewer::isAttached( VVI* vvi) const { return _iman->interactors().count(vvi) > 0;}

// public
bool VtkActorViewer::attach( VVI* vvi)
{
    if (isAttached(vvi))
        return false;
    _iman->addInteractor(vvi);
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
size_t VtkActorViewer::transferInteractors( VtkActorViewer* tv)
{
    if ( tv == this)
        return 0;
    std::unordered_set<VVI*> interactors = _iman->interactors();    // Copy out since moving
    for ( VVI* vvi : interactors)
    {
        detach(vvi);
        tv->attach(vvi);
    }   // end for
    return interactors.size();
}   // end transferInteractors


// protected
void VtkActorViewer::keyPressEvent( QKeyEvent* event)
{
    bool accepted = false;
    for ( KeyPressHandler* kph : _keyPressHandlers)
        accepted |= kph->handleKeyPress(event);
    if ( !accepted)
        QVTKWidget::keyPressEvent( event);
        //QVTKOpenGLWidget::keyPressEvent( event);
}   // end keyPressEvent


// protected
void VtkActorViewer::keyReleaseEvent( QKeyEvent* event)
{
    bool accepted = false;
    for ( KeyPressHandler* kph : _keyPressHandlers)
        accepted |= kph->handleKeyRelease(event);
    if ( !accepted)
        QVTKWidget::keyReleaseEvent( event);
        //QVTKOpenGLWidget::keyReleaseEvent( event);
}   // end keyReleaseEvent
