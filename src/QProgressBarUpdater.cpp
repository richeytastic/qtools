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

#include "QProgressBarUpdater.h"
using QTools::QProgressBarUpdater;
#include <algorithm>


QProgressBarUpdater::QProgressBarUpdater( QProgressBar* bar, int numThreads)
    : QObject(), ProgressDelegate(numThreads), _pbar(bar), _complete(false)
{
    connect( this, SIGNAL( updated(int)), _pbar, SLOT( setValue(int)));
}   // end ctor


void QProgressBarUpdater::reset()
{
    _pbar->reset();
    _pbar->setValue(0);
    _complete = false;
}   // end reset


// protected virtual (called from inside critical section)
void QProgressBarUpdater::processUpdate( float propComplete)
{
    if ( _complete)
        return;

    const int pcnt = 100*propComplete;
    emit updated(pcnt); // Cause update on the GUI thread
    if ( pcnt >= 100)
    {
        emit progressComplete();
        _complete = true;
    }   // end if
}   // end processUpdate


