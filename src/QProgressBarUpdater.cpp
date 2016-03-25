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


