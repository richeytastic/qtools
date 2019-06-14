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

#include <VtkViewerInteractorManager.h>
#include <VtkActorViewer.h>

/*
// For getMouseCoords
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
*/

#include <QApplication>
#include <QDateTime>
#include <algorithm>
#include <cassert>
#include <functional>
#include <chrono>   // to seed rng
using QTools::VtkViewerInteractorManager;
using QTools::VtkActorViewer;
using QTools::InteractionMode;
using QTools::VVI;
using QTools::VMH;


// public
VtkViewerInteractorManager::VtkViewerInteractorManager( VtkActorViewer *qv)
    : _qviewer(qv), _lbdown(false), _rbdown(false), _mbdown(false), _lbDownTime(0),
      _rng( std::chrono::system_clock::now().time_since_epoch().count())
{
    assert(qv);
    _iswitch->setDelegate(this);
    _qviewer->setInteractor( _iswitch);
    setInteractionMode( CAMERA_INTERACTION, false);
}   // end ctor


void VtkViewerInteractorManager::setInteractionMode( InteractionMode m, bool v)
{
    _imode = m;
    if (_imode == CAMERA_INTERACTION)
        _iswitch->setTrackballCamera();
    else
        _iswitch->setTrackballActor();
    _iswitch->setUseCameraOffActor(v);
}   // end setInteractionMode


void VtkViewerInteractorManager::addInteractor( VVI* iface)
{
    assert(iface);
    _vvis.insert(iface);
    if ( iface->keyPressHandler())
        _qviewer->attachKeyPressHandler( iface->keyPressHandler());
}   // end addInteractor

void VtkViewerInteractorManager::removeInteractor( VVI* iface)
{
    assert(iface);
    _qviewer->detachKeyPressHandler( iface->keyPressHandler());
    _vvis.erase(iface);
}   // end removeInteractor


void VtkViewerInteractorManager::addMouseHandler( VMH* mh)
{
    assert(mh);
    _vmhs.insert(mh);
}   // end addMouseHandler

void VtkViewerInteractorManager::removeMouseHandler( VMH* mh)
{
    assert(mh);
    _vmhs.erase(mh);
}   // end removeMouseHandler


int VtkViewerInteractorManager::lockInteraction()
{
    int lkey = (int)_rng();             // Don't care about value being unsigned so cast is fine
    while ( _lockKeys.count(lkey) > 0)  // HAHAHA!!! Really, this is just soooo implausible (P(1/(2^32)) if non-biased RNG).
        lkey = (int)_rng();
    _lockKeys.insert(lkey);
    return lkey;
}   // end lockInteraction


bool VtkViewerInteractorManager::unlockInteraction( int lkey)
{
    _lockKeys.erase(lkey);
    return !isInteractionLocked();
}   // end unlockInteraction


bool VtkViewerInteractorManager::isInteractionLocked() const { return !_lockKeys.empty();}

/*
// VTK 2D origin is at bottom left of render window so need to set to top left.
void VtkViewerInteractorManager::updateMouseCoords()
{
    vtkRenderWindowInteractor* rint = _iswitch->GetInteractor();
    const int xpos = rint->GetEventPosition()[0];
    const int ypos = rint->GetEventPosition()[1];
    vtkRenderer* ren = rint->FindPokedRenderer( xpos, ypos);
    _mPos.rx() = xpos;
    _mPos.ry() = ren->GetSize()[1] - ypos - 1;   // Inversion of mouse coords
}   // end updateMouseCoords
*/


namespace {

bool dofunction( const std::unordered_set<VVI*>& vvis, std::function<bool(VVI*)> func)
{
    bool swallowed = false;
    for ( VVI* vvi : vvis)
    {
        if ( vvi->isEnabled())
            swallowed |= func(vvi);
    }   // end for
    return swallowed;
}   // end dofunction

bool dofunction( const std::unordered_set<VMH*>& vmhs, std::function<bool(VMH*)> func)
{
    bool swallowed = false;
    for ( VMH* vmh : vmhs)
    {
        if ( vmh->isEnabled())
            swallowed |= func(vmh);
    }   // end for
    return swallowed;
}   // end dofunction

void docamera( const std::unordered_set<VVI*>& vvis, std::function<void(VVI*)> func)
{
    for ( VVI* vvi : vvis)
    {
        if ( vvi->isEnabled())
            func(vvi);
    }   // end for
}   // end docamera

}   // end namespace



bool VtkViewerInteractorManager::doOnLeftButtonDown()
{
    _lbdown = true;
    bool swallowed = false;
    const qint64 tnow = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    if ( (tnow - _lbDownTime) < QApplication::doubleClickInterval())    // Check for double click
    {
        _lbDownTime = 0;
        swallowed = dofunction( _vmhs, [this](VMH* v){ return v->leftDoubleClick();});
    }   // end if
    else
    {
        _lbDownTime = tnow;
        swallowed = dofunction( _vmhs, [this](VMH* v){ return v->leftButtonDown();});
    }   // end else
    return swallowed;
}   // end doOnLeftButtonDown


bool VtkViewerInteractorManager::doOnLeftButtonUp()
{
    _lbdown = false;
    return dofunction( _vmhs, [this](VMH* v){ return v->leftButtonUp();});
}   // end doOnLeftButtonUp


bool VtkViewerInteractorManager::doOnRightButtonDown()
{
    _rbdown = true;
    return dofunction( _vmhs, [this](VMH* v){ return v->rightButtonDown();});
}   // end doOnRightButtonDown


bool VtkViewerInteractorManager::doOnRightButtonUp()
{
    _rbdown = false;
    return dofunction( _vmhs, [this](VMH* v){ return v->rightButtonUp();});
}   // end doOnRightButtonUp


bool VtkViewerInteractorManager::doOnMiddleButtonDown()
{
    _mbdown = true;
    return dofunction( _vmhs, [this](VMH* v){ return v->middleButtonDown();});
}   // end doOnMiddleButtonDown


bool VtkViewerInteractorManager::doOnMiddleButtonUp()
{
    _mbdown = false;
    return dofunction( _vmhs, [this](VMH* v){ return v->middleButtonUp();});
}   // end doOnMiddleButtonUp


bool VtkViewerInteractorManager::doOnMouseWheelForward()
{
    return dofunction( _vmhs, [this](VMH* v){ return v->mouseWheelForward();});
}   // end doOnMouseWheelForward


bool VtkViewerInteractorManager::doOnMouseWheelBackward()
{
    return dofunction( _vmhs, [this](VMH* v){ return v->mouseWheelBackward();});
}   // end doOnMouseWheelBackward


bool VtkViewerInteractorManager::doOnMouseMove()
{
    //updateMouseCoords();
    bool swallowed = false;
    if ( _lbdown)
        swallowed = dofunction( _vmhs, [this](VMH* v){ return v->leftDrag();});
    else if ( _rbdown)
        swallowed = dofunction( _vmhs, [this](VMH* v){ return v->rightDrag();});
    else if ( _mbdown)
        swallowed = dofunction( _vmhs, [this](VMH* v){ return v->middleDrag();});
    else
        swallowed = dofunction( _vmhs, [this](VMH* v){ return v->mouseMove();});
    return swallowed;
}   // end doOnMouseMove


void VtkViewerInteractorManager::doOnEnter()
{
    //updateMouseCoords();
    dofunction( _vvis, [this](VVI* v){ v->mouseEnter( _qviewer); return false;});
    dofunction( _vmhs, [this](VMH* v){ v->mouseEnter( _qviewer); return false;});
}   // end doOnEnter

void VtkViewerInteractorManager::doOnLeave()
{
    dofunction( _vvis, [this](VVI* v){ v->mouseLeave( _qviewer); return false;});
    dofunction( _vmhs, [this](VMH* v){ v->mouseLeave( _qviewer); return false;});
}   // end doOnLeave


void VtkViewerInteractorManager::doAfterCameraRotate() { docamera( _vvis, [](VVI* v){ v->cameraRotate(); v->cameraMove();});}
void VtkViewerInteractorManager::doAfterCameraDolly() { docamera( _vvis, [](VVI* v){ v->cameraDolly(); v->cameraMove();});}
void VtkViewerInteractorManager::doAfterCameraSpin() { docamera( _vvis, [](VVI* v){ v->cameraSpin(); v->cameraMove();});}
void VtkViewerInteractorManager::doAfterCameraPan() { docamera( _vvis, [](VVI* v){ v->cameraPan(); v->cameraMove();});}

void VtkViewerInteractorManager::doBeforeCameraStart() { docamera( _vvis, [](VVI* v){ v->cameraStart();});}
void VtkViewerInteractorManager::doAfterCameraStop() { docamera( _vvis, [](VVI* v){ v->cameraStop();});}

void VtkViewerInteractorManager::doAfterActorRotate( const vtkProp3D* p) { docamera( _vvis, [=](VVI* v){ v->actorRotate(p); v->actorMove(p);});}
void VtkViewerInteractorManager::doAfterActorDolly( const vtkProp3D* p) { docamera( _vvis, [=](VVI* v){ v->actorDolly(p); v->actorMove(p);});}
void VtkViewerInteractorManager::doAfterActorSpin( const vtkProp3D* p) { docamera( _vvis, [=](VVI* v){ v->actorSpin(p); v->actorMove(p);});}
void VtkViewerInteractorManager::doAfterActorPan( const vtkProp3D* p) { docamera( _vvis, [=](VVI* v){ v->actorPan(p); v->actorMove(p);});}

void VtkViewerInteractorManager::doBeforeActorStart( const vtkProp3D* p) { docamera( _vvis, [=](VVI* v){ v->actorStart(p);});}
void VtkViewerInteractorManager::doAfterActorStop( const vtkProp3D* p) { docamera( _vvis, [=](VVI* v){ v->actorStop(p);});}
