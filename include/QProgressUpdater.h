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

/**
 * Allow calls to update a progress bar from outside of the GUI thread.
 * Progress bar not required, can be used simply as an emitter of progress updates.
 */

#ifndef QPROGRESS_UPDATER_H
#define QPROGRESS_UPDATER_H

#include "QTools_Export.h"
#include <QProgressBar>
#include <ProgressDelegate.h>   // rlib

namespace QTools
{

class QTools_EXPORT QProgressUpdater : public QObject, public rlib::ProgressDelegate
{ Q_OBJECT
public:
    QProgressUpdater( QProgressBar* bar=NULL, int numThreads=1);
    void reset();   // Reset complete flag (and the progress bar if set)

    virtual void processUpdate( float propComplete);    // Called inside critical section

signals:
    void progressUpdated( float propComplete);  // propComplete in [0,1]
    void progressComplete();    // Emitted at >= 100% (emitted at least once!)

private:
    QProgressBar* _pbar;
    bool _complete;
};  // end class

}   // end namespace


#endif


