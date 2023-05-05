/************************************************************************
 * Copyright (C) 2023 Richard Palmer
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

#include <QDebug>
#include <QString>
#include <QProcess>
#include <FdToolProcess.h>
using QTools::FdToolProcess;


void FdToolProcess::_init()
{
    _waitToKillMsecs = 5000;
    _waitForStartMsecs = 20000;
    _waitForImgReadMsecs = 3000;
    _fdtool.setProcessChannelMode( QProcess::MergedChannels);   // Merge stderr and stdout
}   // end _init


FdToolProcess::FdToolProcess() { _init();}
FdToolProcess::FdToolProcess( const QString &fdToolPath) : _fdtoolPath(fdToolPath) { _init();}
    
FdToolProcess::~FdToolProcess() { stop();}


bool FdToolProcess::start( const QString &fdToolPath)
{
    _fdtoolPath = fdToolPath;
    return start();
}   // end start


bool FdToolProcess::start()
{
    if ( isStarted())
        return false;
    _fdtool.start( _fdtoolPath);
    bool startedOkay = false;
    qInfo() << "Initialising " << _fdtoolPath << "...";
    if ( _fdtool.waitForStarted( _waitForStartMsecs))
    {
        if ( _fdtool.waitForReadyRead( _waitForStartMsecs))
        {
            qInfo() << _fdtoolPath << " started (process id = " << _fdtool.processId() << ")";
            qInfo() << _fdtool.readAllStandardOutput();
            startedOkay = true;
        }   // end if
    }   // end if
    return startedOkay;
}   // end start


bool FdToolProcess::stop()
{
    if ( !isStarted())
        return false;
    _fdtool.write( QString("SHUTDOWN\n").toUtf8());
    _fdtool.waitForBytesWritten( 100);
    if ( !_fdtool.waitForFinished(_waitToKillMsecs))    // Kill if not stopped within short duration
    {
        qWarning() << "Killing " << _fdtoolPath << "...";
        _fdtool.kill();
    }   // end if
    return true;
}   // end stop


bool FdToolProcess::isStarted() const { return _fdtool.processId() != 0;}


QString FdToolProcess::process( const QString &pathToImage)
{
    QString outstr;
    _fdtool.write( (pathToImage + "\n").toUtf8());
    _fdtool.waitForBytesWritten( 100);
    if ( _fdtool.waitForReadyRead( _waitForImgReadMsecs))
        outstr = _fdtool.readAllStandardOutput();
    return outstr;
}   // end process
