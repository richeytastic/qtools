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

#include <QProgressUpdater.h>
using QTools::QProgressUpdater;
#include <algorithm>

QProgressUpdater::Ptr QProgressUpdater::create( QProgressBar* pb, int nt)
{
    return Ptr( new QProgressUpdater( pb, nt));
}   // end create


// private
QProgressUpdater::QProgressUpdater( QProgressBar* pbar, int numThreads)
    : ProgressDelegate(numThreads), _pbar(pbar), _complete(false)
{
}   // end ctor


void QProgressUpdater::reset()
{
    if ( _pbar)
        _pbar->reset();
    _complete = false;
}   // end reset


// protected virtual (called from inside critical section)
void QProgressUpdater::processUpdate( float propComplete)
{
    if ( _complete)
        return;

    propComplete = std::max( 0.0f, std::min( 1.0f, propComplete));
    if ( _pbar)
        _pbar->setValue( _pbar->maximum()*propComplete);
    emit progressUpdated( propComplete); // Cause update on the GUI thread

    if ( propComplete >= 1.0f)
    {
        emit progressComplete();
        _complete = true;
    }   // end if
}   // end processUpdate

