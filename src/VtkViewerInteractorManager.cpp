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

// For getMouseCoords
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>

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


// public
VtkViewerInteractorManager::VtkViewerInteractorManager( VtkActorViewer *qv)
    : _qviewer(qv), _mousePos(0,0), _onRenderer(false), _lbdown(false), _rbdown(false), _mbdown(false), _lbDownTime(0),
      _rng( std::chrono::system_clock::now().time_since_epoch().count()), _istyle(nullptr)
{
    assert(qv);
    _istyle = VtkViewerSwitchInteractor::New();
    _istyle->setDelegate(this);
    _qviewer->setInteractor( _istyle);
    setInteractionMode( CAMERA_INTERACTION);
}   // end ctor


// public
VtkViewerInteractorManager::~VtkViewerInteractorManager()
{
    _istyle->Delete();
}   // end dtor


void VtkViewerInteractorManager::setInteractionMode( InteractionMode m)
{
    _imode = m;
    if (_imode == CAMERA_INTERACTION)
        _istyle->setTrackballCamera();
    else
        _istyle->setTrackballActor();
}   // end setInteractionMode


const std::unordered_set<VVI*>& VtkViewerInteractorManager::interactors() const { return _vvis;}

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


// VTK 2D origin is at bottom left of render window so need to set to top left.
void VtkViewerInteractorManager::updateMouseCoords()
{
    vtkRenderWindowInteractor* rint = _istyle->GetInteractor();
    vtkRenderer* ren = rint->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    _mousePos.rx() = rint->GetEventPosition()[0];
    _mousePos.ry() = ren->GetSize()[1] - rint->GetEventPosition()[1] - 1;
}   // end updateMouseCoords


namespace {
bool dofunction( const std::unordered_set<VVI*>& vvis, std::function<bool(VVI*)> func)
{
    // If vvis are instead in a priority queue then the highest priority interactor can be
    // handled first and then no other interactors should be called to handle the interaction.
    bool swallowed = false;
    for ( VVI* vvi : vvis)
    {
        swallowed |= vvi->isEnabled() && func(vvi);
    }   // end for
    return swallowed;
}   // end dofunction
}   // end namespace



bool VtkViewerInteractorManager::doOnLeftButtonDown()
{
    _lbdown = true;
    bool swallowed = false;
    const qint64 tnow = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    if ( (tnow - _lbDownTime) < QApplication::doubleClickInterval())    // Check for double click
    {
        _lbDownTime = 0;
        swallowed = dofunction( _vvis, [this](VVI* vvi){ return vvi->leftDoubleClick(_mousePos);});
    }   // end if
    else
    {
        _lbDownTime = tnow;
        swallowed = dofunction( _vvis, [this](VVI* vvi){ return vvi->leftButtonDown(_mousePos);});
    }   // end else
    return swallowed;
}   // end doOnLeftButtonDown


bool VtkViewerInteractorManager::doOnLeftButtonUp()
{
    _lbdown = false;
    return dofunction( _vvis, [this](VVI* vvi){ return vvi->leftButtonUp(_mousePos);});
}   // end doOnLeftButtonUp


bool VtkViewerInteractorManager::doOnRightButtonDown()
{
    _rbdown = true;
    return dofunction( _vvis, [this](VVI* vvi){ return vvi->rightButtonDown(_mousePos);});
}   // end doOnRightButtonDown


bool VtkViewerInteractorManager::doOnRightButtonUp()
{
    _rbdown = false;
    return dofunction( _vvis, [this](VVI* vvi){ return vvi->rightButtonUp(_mousePos);});
}   // end doOnRightButtonUp


bool VtkViewerInteractorManager::doOnMiddleButtonDown()
{
    _mbdown = true;
    return dofunction( _vvis, [this](VVI* vvi){ return vvi->middleButtonDown(_mousePos);});
}   // end doOnMiddleButtonDown


bool VtkViewerInteractorManager::doOnMiddleButtonUp()
{
    _mbdown = false;
    return dofunction( _vvis, [this](VVI* vvi){ return vvi->middleButtonUp(_mousePos);});
}   // end doOnMiddleButtonUp


bool VtkViewerInteractorManager::doOnMouseWheelForward()
{
    return dofunction( _vvis, [this](VVI* vvi){ return vvi->mouseWheelForward(_mousePos);});
}   // end doOnMouseWheelForward


bool VtkViewerInteractorManager::doOnMouseWheelBackward()
{
    return dofunction( _vvis, [this](VVI* vvi){ return vvi->mouseWheelBackward(_mousePos);});
}   // end doOnMouseWheelBackward


bool VtkViewerInteractorManager::doOnMouseMove()
{
    updateMouseCoords();
    bool swallowed = false;
    if ( _lbdown)
        swallowed = dofunction( _vvis, [this](VVI* vvi){ return vvi->leftDrag(_mousePos);});
    else if ( _rbdown)
        swallowed = dofunction( _vvis, [this](VVI* vvi){ return vvi->rightDrag(_mousePos);});
    else if ( _mbdown)
        swallowed = dofunction( _vvis, [this](VVI* vvi){ return vvi->middleDrag(_mousePos);});
    else
        swallowed = dofunction( _vvis, [this](VVI* vvi){ return vvi->mouseMove(_mousePos);});
    return swallowed;
}   // end doOnMouseMove


bool VtkViewerInteractorManager::doOnEnter()
{
    _onRenderer = true;
    updateMouseCoords();
    return dofunction( _vvis, [this](VVI* vvi){ return vvi->mouseEnter(_mousePos);});
}   // end doOnEnter

bool VtkViewerInteractorManager::doOnLeave()
{
    _onRenderer = false;
    return dofunction( _vvis, [this](VVI* vvi){ return vvi->mouseLeave(_mousePos);});
}   // end doOnLeave


namespace {
void docamera( const std::unordered_set<VVI*>& vvis, std::function<void(VVI*)> func)
{
    for ( VVI* vvi : vvis)
        func(vvi);
}   // end docamera
}   // end namespace


void VtkViewerInteractorManager::doAfterCameraRotate() { docamera( _vvis, [](VVI* vvi){ vvi->cameraRotate();});}
void VtkViewerInteractorManager::doAfterCameraDolly() { docamera( _vvis, [](VVI* vvi){ vvi->cameraDolly();});}
void VtkViewerInteractorManager::doAfterCameraSpin() { docamera( _vvis, [](VVI* vvi){ vvi->cameraSpin();});}
void VtkViewerInteractorManager::doAfterCameraPan() { docamera( _vvis, [](VVI* vvi){ vvi->cameraPan();});}
void VtkViewerInteractorManager::doAfterCameraStop() { docamera( _vvis, [](VVI* vvi){ vvi->cameraStop();});}

void VtkViewerInteractorManager::doAfterActorRotate() { docamera( _vvis, [](VVI* vvi){ vvi->actorRotate();});}
void VtkViewerInteractorManager::doAfterActorDolly() { docamera( _vvis, [](VVI* vvi){ vvi->actorDolly();});}
void VtkViewerInteractorManager::doAfterActorSpin() { docamera( _vvis, [](VVI* vvi){ vvi->actorSpin();});}
void VtkViewerInteractorManager::doAfterActorPan() { docamera( _vvis, [](VVI* vvi){ vvi->actorPan();});}
void VtkViewerInteractorManager::doAfterActorStop() { docamera( _vvis, [](VVI* vvi){ vvi->actorStop();});}
