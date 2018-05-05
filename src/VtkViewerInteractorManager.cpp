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
#include <VtkViewerActorInteractor.h>
#include <VtkViewerCameraInteractor.h>
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
using QTools::VtkViewerInteractorManager;
using QTools::VtkActorViewer;
using QTools::InteractionMode;
using QTools::VVI;


// public
VtkViewerInteractorManager::VtkViewerInteractorManager( VtkActorViewer *qv)
    : _qviewer(qv), _locked(false), _lbdown(false), _rbdown(false), _mbdown(false), _lbDownTime(0)
{
    assert(qv);

    vtkSmartPointer<VtkViewerActorInteractor> ai = vtkSmartPointer<VtkViewerActorInteractor>::New();
    ai->setDelegate(this);
    _interactors[ACTOR_INTERACTION] = ai;

    vtkSmartPointer<VtkViewerCameraInteractor> ci = vtkSmartPointer<VtkViewerCameraInteractor>::New();
    ci->setDelegate(this);
    _interactors[CAMERA_INTERACTION] = ci;

    setInteractionMode( CAMERA_INTERACTION);
}   // end ctor


void VtkViewerInteractorManager::setInteractionMode( InteractionMode m)
{
    _imode = m;
    assert( _interactors.count(_imode) > 0);
    _qviewer->setInteractor( _interactors.at(m));
}   // end setInteractionMode


const std::unordered_set<VVI*>& VtkViewerInteractorManager::interactors() const { return _ifaces;}

void VtkViewerInteractorManager::addInteractor( VVI* iface)
{
    _ifaces.insert(iface);
    if ( iface->keyPressHandler())
        _qviewer->attachKeyPressHandler( iface->keyPressHandler());
}   // end addInteractor

void VtkViewerInteractorManager::removeInteractor( VVI* iface)
{
    _ifaces.erase(iface);
    _qviewer->detachKeyPressHandler( iface->keyPressHandler());
}   // end removeInteractor

void VtkViewerInteractorManager::setInteractionLocked( bool v) { _locked = v;}
bool VtkViewerInteractorManager::isInteractionLocked() const { return _locked;}


// VTK 2D origin is at bottom left of render window so need to set to top left.
QPoint VtkViewerInteractorManager::getMouseCoords()
{
    vtkSmartPointer<vtkInteractorStyle> istyle = _interactors.at(_imode);
    vtkRenderWindowInteractor* rint = istyle->GetInteractor();
    vtkSmartPointer<vtkRenderer> ren = rint->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    return QPoint( rint->GetEventPosition()[0], ren->GetSize()[1] - rint->GetEventPosition()[1] - 1);
}   // end getMouseCoords


void VtkViewerInteractorManager::doOnLeftButtonDown()
{
    const QPoint p = getMouseCoords();
    _lbdown = true;
    const qint64 tnow = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    if ( (tnow - _lbDownTime) < QApplication::doubleClickInterval())    // A second left down received inside double click interval
    {
        _lbDownTime = 0;
        std::for_each( std::begin(_ifaces), std::end(_ifaces), [&](VVI* vvi){vvi->leftDoubleClick(p);});
    }   // end if
    else
    {
        _lbDownTime = tnow;
        std::for_each( std::begin(_ifaces), std::end(_ifaces), [&](VVI* vvi){vvi->leftButtonDown(p);});
    }   // end else
}   // end doOnLeftButtonDown


void VtkViewerInteractorManager::doOnLeftButtonUp()
{
    const QPoint p = getMouseCoords();
    _lbdown = false;
    if ( _lbDownTime > 0)   // Ensure left button up only after single click down (not after double click)
        std::for_each( std::begin(_ifaces), std::end(_ifaces), [&](VVI* vvi){vvi->leftButtonUp(p);});
}   // end doOnLeftButtonUp


void VtkViewerInteractorManager::doOnRightButtonDown()
{
    const QPoint p = getMouseCoords();
    _rbdown = true;
    std::for_each( std::begin(_ifaces), std::end(_ifaces), [&](VVI* vvi){vvi->rightButtonDown(p);});
}   // end doOnRightButtonDown


void VtkViewerInteractorManager::doOnRightButtonUp()
{
    const QPoint p = getMouseCoords();
    _rbdown = false;
    std::for_each( std::begin(_ifaces), std::end(_ifaces), [&](VVI* vvi){vvi->rightButtonUp(p);});
}   // end doOnRightButtonUp


void VtkViewerInteractorManager::doOnMiddleButtonDown()
{
    const QPoint p = getMouseCoords();
    _mbdown = true;
    std::for_each( std::begin(_ifaces), std::end(_ifaces), [&](VVI* vvi){vvi->middleButtonDown(p);});
}   // end doOnMiddleButtonDown


void VtkViewerInteractorManager::doOnMiddleButtonUp()
{
    const QPoint p = getMouseCoords();
    _mbdown = false;
    std::for_each( std::begin(_ifaces), std::end(_ifaces), [&](VVI* vvi){vvi->middleButtonUp(p);});
}   // end doOnMiddleButtonUp


void VtkViewerInteractorManager::doOnMouseWheelForward()
{
    const QPoint p = getMouseCoords();
    std::for_each( std::begin(_ifaces), std::end(_ifaces), [&](VVI* vvi){vvi->mouseWheelForward(p);});
}   // end doOnMouseWheelForward


void VtkViewerInteractorManager::doOnMouseWheelBackward()
{
    const QPoint p = getMouseCoords();
    std::for_each( std::begin(_ifaces), std::end(_ifaces), [&](VVI* vvi){vvi->mouseWheelBackward(p);});
}   // end doOnMouseWheelBackward


void VtkViewerInteractorManager::doOnMouseMove()
{
    const QPoint p = getMouseCoords();
    for ( VVI* vvi : _ifaces)
    {
        if ( _lbdown)
            vvi->leftDrag(p);
        else if ( _rbdown)
            vvi->rightDrag(p);
        else if ( _mbdown)
            vvi->middleDrag(p);
        else
            vvi->mouseMove(p);
    }   // end for
}   // end doOnMouseMove


void VtkViewerInteractorManager::doOnEnter()
{
    const QPoint p = getMouseCoords();
    std::for_each( std::begin(_ifaces), std::end(_ifaces), [&](VVI* vvi){vvi->mouseEnter(p);});
}   // end doOnEnter

void VtkViewerInteractorManager::doOnLeave()
{
    const QPoint p = getMouseCoords();
    std::for_each( std::begin(_ifaces), std::end(_ifaces), [&](VVI* vvi){vvi->mouseLeave(p);});
}   // end doOnLeave

