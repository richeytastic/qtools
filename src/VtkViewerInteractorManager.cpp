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
    : _qviewer(qv), _lbdown(false), _rbdown(false), _mbdown(false), _lbDownTime(0),
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
    _vvis.insert(iface);
    if ( iface->keyPressHandler())
        _qviewer->attachKeyPressHandler( iface->keyPressHandler());
}   // end addInteractor

void VtkViewerInteractorManager::removeInteractor( VVI* iface)
{
    _vvis.erase(iface);
    _qviewer->detachKeyPressHandler( iface->keyPressHandler());
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
QPoint VtkViewerInteractorManager::getMouseCoords()
{
    vtkRenderWindowInteractor* rint = _istyle->GetInteractor();
    vtkSmartPointer<vtkRenderer> ren = rint->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    return QPoint( rint->GetEventPosition()[0], ren->GetSize()[1] - rint->GetEventPosition()[1] - 1);
}   // end getMouseCoords


namespace {
bool dofunction( const std::unordered_set<VVI*>& ifaces, std::function<bool(VVI*)> func)
{
    bool swallowed = false;
    for ( VVI* vvi : ifaces)
    {
        const bool enabled = vvi->isEnabled();
        if ( enabled && func(vvi))
            swallowed = true;
    }   // end for
    return swallowed;
}   // end dofunction
}   // end namespace



bool VtkViewerInteractorManager::doOnLeftButtonDown()
{
    const QPoint p = getMouseCoords();
    _lbdown = true;
    bool swallowed = false;
    const qint64 tnow = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    if ( (tnow - _lbDownTime) < QApplication::doubleClickInterval())    // Check for double click
    {
        _lbDownTime = 0;
        swallowed = dofunction( _vvis, [&p](VVI* vvi){ return vvi->leftDoubleClick(p);});
    }   // end if
    else
    {
        _lbDownTime = tnow;
        swallowed = dofunction( _vvis, [&p](VVI* vvi){ return vvi->leftButtonDown(p);});
    }   // end else
    return swallowed;
}   // end doOnLeftButtonDown


bool VtkViewerInteractorManager::doOnLeftButtonUp()
{
    const QPoint p = getMouseCoords();
    _lbdown = false;
    bool swallowed = dofunction( _vvis, [&p](VVI* vvi){ return vvi->leftButtonUp(p);});
    return swallowed;
}   // end doOnLeftButtonUp


bool VtkViewerInteractorManager::doOnRightButtonDown()
{
    const QPoint p = getMouseCoords();
    _rbdown = true;
    return dofunction( _vvis, [&p](VVI* vvi){ return vvi->rightButtonDown(p);});
}   // end doOnRightButtonDown


bool VtkViewerInteractorManager::doOnRightButtonUp()
{
    const QPoint p = getMouseCoords();
    _rbdown = false;
    return dofunction( _vvis, [&p](VVI* vvi){ return vvi->rightButtonUp(p);});
}   // end doOnRightButtonUp


bool VtkViewerInteractorManager::doOnMiddleButtonDown()
{
    const QPoint p = getMouseCoords();
    _mbdown = true;
    return dofunction( _vvis, [&p](VVI* vvi){ return vvi->middleButtonDown(p);});
}   // end doOnMiddleButtonDown


bool VtkViewerInteractorManager::doOnMiddleButtonUp()
{
    const QPoint p = getMouseCoords();
    _mbdown = false;
    return dofunction( _vvis, [&p](VVI* vvi){ return vvi->middleButtonUp(p);});
}   // end doOnMiddleButtonUp


bool VtkViewerInteractorManager::doOnMouseWheelForward()
{
    const QPoint p = getMouseCoords();
    return dofunction( _vvis, [&p](VVI* vvi){ return vvi->mouseWheelForward(p);});
}   // end doOnMouseWheelForward


bool VtkViewerInteractorManager::doOnMouseWheelBackward()
{
    const QPoint p = getMouseCoords();
    return dofunction( _vvis, [&p](VVI* vvi){ return vvi->mouseWheelBackward(p);});
}   // end doOnMouseWheelBackward


bool VtkViewerInteractorManager::doOnMouseMove()
{
    const QPoint p = getMouseCoords();
    bool swallowed = false;
    if ( _lbdown)
        swallowed = dofunction( _vvis, [&p](VVI* vvi){ return vvi->leftDrag(p);});
    else if ( _rbdown)
        swallowed = dofunction( _vvis, [&p](VVI* vvi){ return vvi->rightDrag(p);});
    else if ( _mbdown)
        swallowed = dofunction( _vvis, [&p](VVI* vvi){ return vvi->middleDrag(p);});
    else
        swallowed = dofunction( _vvis, [&p](VVI* vvi){ return vvi->mouseMove(p);});
    return swallowed;
}   // end doOnMouseMove


bool VtkViewerInteractorManager::doOnEnter()
{
    const QPoint p = getMouseCoords();
    return dofunction( _vvis, [&p](VVI* vvi){ return vvi->mouseEnter(p);});
}   // end doOnEnter

bool VtkViewerInteractorManager::doOnLeave()
{
    const QPoint p = getMouseCoords();
    return dofunction( _vvis, [&p](VVI* vvi){ return vvi->mouseLeave(p);});
}   // end doOnLeave


namespace {
void docamera( const std::unordered_set<VVI*>& ifaces, std::function<void(VVI*)> func)
{
    for ( VVI* vvi : ifaces)
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


