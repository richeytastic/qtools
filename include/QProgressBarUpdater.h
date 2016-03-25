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


