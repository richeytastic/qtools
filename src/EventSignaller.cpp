/************************************************************************
 * Copyright (C) 2021 Richard Palmer
 *
 * Cliniface is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cliniface is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#include <QTools/EventSignaller.h>
#include <cassert>
using QTools::EventSignaller;


EventSignaller::EventSignaller( QEvent::Type et, bool swallow)
    : _evType(et), _doSwallow(swallow) {}


void EventSignaller::install( QObject *instObj)
{
    assert(instObj);
    instObj->installEventFilter(this);
}   // end install


bool EventSignaller::eventFilter( QObject *obj, QEvent *evt)
{
    if ( evt->type() == _evType)
    {
        emit onEvent( obj);
        return _doSwallow;
    }   // end if
    return false;
}   // end eventFilter
