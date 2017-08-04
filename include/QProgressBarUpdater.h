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
 */

#pragma once
#ifndef QPROGRESS_BAR_UPDATER_H
#define QPROGRESS_BAR_UPDATER_H

#include "QTools_Export.h"
#include <QProgressBar>
#include <ProgressDelegate.h>
using rlib::ProgressDelegate;

namespace QTools
{

class QTools_EXPORT QProgressBarUpdater : public QObject, public ProgressDelegate
{ Q_OBJECT
public:
    QProgressBarUpdater( QProgressBar* bar, int numThreads);
    void reset();

protected:
    virtual void processUpdate( float propComplete);    // Called inside critical section

signals:
    void updated( int percentComplete);
    void progressComplete();    // Emitted at >= 100%

private:
    QProgressBar* _pbar;
    bool _complete;
};  // end class

}   // end namespace


#endif


