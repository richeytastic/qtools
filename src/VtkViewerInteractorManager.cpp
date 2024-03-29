/************************************************************************
 * Copyright (C) 2022 Richard Palmer
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


VtkViewerInteractorManager::VtkViewerInteractorManager( VtkActorViewer *qv)
    : _qviewer(qv), _lbdown(false), _rbdown(false), _mbdown(false),
      _rng( std::chrono::system_clock::now().time_since_epoch().count())
{
    assert(qv);
    _iswitch->setDelegate(this);
    _qviewer->setInteractor( _iswitch);
    setCameraInteraction();
}   // end ctor


void VtkViewerInteractorManager::setCameraInteraction()
{
    _imode = CAMERA_INTERACTION;
    _iswitch->setTrackballCamera();
}   // end setCameraInteraction


void VtkViewerInteractorManager::setActorInteraction( const vtkProp3D *p)
{
    _imode = ACTOR_INTERACTION;
    _iswitch->setTrackballActor( p);
}   // end setActorInteraction


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


namespace {

bool dofunction( const std::unordered_set<VVI*>& vvis, const std::function<bool(VVI*)> &func)
{
    bool swallowed = false;
    for ( VVI* vvi : vvis)
    {
        if ( vvi->isEnabled())
            swallowed |= func(vvi);
        if ( swallowed)
            break;
    }   // end for
    return swallowed;
}   // end dofunction

bool dofunction( const std::unordered_set<VMH*>& vmhs, const std::function<bool(VMH*)> &func)
{
    bool swallowed = false;
    for ( VMH* vmh : vmhs)
    {
        if ( vmh->isEnabled())
            swallowed |= func(vmh);
        if ( swallowed)
            break;
    }   // end for
    return swallowed;
}   // end dofunction

void docamera( const std::unordered_set<VVI*>& vvis, const std::function<void(VVI*)> &func)
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
    bool swallowed = dofunction( _vmhs, [](VMH* v){ return v->leftButtonDown();});
    _qviewer->updateRender();
    return swallowed;
}   // end doOnLeftButtonDown


bool VtkViewerInteractorManager::doOnLeftButtonDoubleClick()
{
    const bool swallowed = dofunction( _vmhs, [](VMH* v){ return v->leftDoubleClick();});
    _qviewer->updateRender();
    return swallowed;
}   // end doOnLeftButtonDoubleClick


bool VtkViewerInteractorManager::doOnLeftButtonUp()
{
    _lbdown = false;
    const bool rval = dofunction( _vmhs, [](VMH* v){ return v->leftButtonUp();});
    _qviewer->updateRender();
    return rval;
}   // end doOnLeftButtonUp


bool VtkViewerInteractorManager::doOnRightButtonDown()
{
    _rbdown = true;
    const bool swallowed = dofunction( _vmhs, [](VMH* v){ return v->rightButtonDown();});
    _qviewer->updateRender();
    return swallowed;
}   // end doOnRightButtonDown


bool VtkViewerInteractorManager::doOnRightButtonDoubleClick()
{
    const bool swallowed = dofunction( _vmhs, [](VMH* v){ return v->rightDoubleClick();});
    _qviewer->updateRender();
    return swallowed;
}   // end doOnRightButtonDoubleClick


bool VtkViewerInteractorManager::doOnRightButtonUp()
{
    _rbdown = false;
    const bool rval = dofunction( _vmhs, [](VMH* v){ return v->rightButtonUp();});
    _qviewer->updateRender();
    return rval;
}   // end doOnRightButtonUp


bool VtkViewerInteractorManager::doOnMiddleButtonDown()
{
    _mbdown = true;
    const bool rval = dofunction( _vmhs, [](VMH* v){ return v->middleButtonDown();});
    _qviewer->updateRender();
    return rval;
}   // end doOnMiddleButtonDown


bool VtkViewerInteractorManager::doOnMiddleButtonUp()
{
    _mbdown = false;
    const bool rval = dofunction( _vmhs, [](VMH* v){ return v->middleButtonUp();});
    _qviewer->updateRender();
    return rval;
}   // end doOnMiddleButtonUp


bool VtkViewerInteractorManager::doOnMouseWheelForward()
{
    const bool rval = dofunction( _vmhs, [](VMH* v){ return v->mouseWheelForward();});
    _qviewer->updateRender();
    return rval;
}   // end doOnMouseWheelForward


bool VtkViewerInteractorManager::doOnMouseWheelBackward()
{
    const bool rval = dofunction( _vmhs, [](VMH* v){ return v->mouseWheelBackward();});
    _qviewer->updateRender();
    return rval;
}   // end doOnMouseWheelBackward


bool VtkViewerInteractorManager::doOnMouseMove()
{
    bool swallowed = false;
    if ( _lbdown)
    {
        swallowed = dofunction( _vmhs, [](VMH* v){ return v->leftDrag();});
        _qviewer->updateRender();
    }   // end if
    else if ( _rbdown)
    {
        swallowed = dofunction( _vmhs, [](VMH* v){ return v->rightDrag();});
        _qviewer->updateRender();
    }   // end else if
    else if ( _mbdown)
    {
        swallowed = dofunction( _vmhs, [](VMH* v){ return v->middleDrag();});
        _qviewer->updateRender();
    }   // end else if
    else
    {
        swallowed = dofunction( _vmhs, [](VMH* v){ return v->mouseMove();});
        _qviewer->updateRender();
    }   // end else if
    return swallowed;
}   // end doOnMouseMove


void VtkViewerInteractorManager::doOnEnter()
{
    dofunction( _vvis, [this](VVI* v){ v->mouseEnter( _qviewer); return false;});
    dofunction( _vmhs, [this](VMH* v){ v->mouseEnter( _qviewer); return false;});
    _qviewer->updateRender();
}   // end doOnEnter

void VtkViewerInteractorManager::doOnLeave()
{
    dofunction( _vvis, [this](VVI* v){ v->mouseLeave( _qviewer); return false;});
    dofunction( _vmhs, [this](VMH* v){ v->mouseLeave( _qviewer); return false;});
    _qviewer->updateRender();
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
