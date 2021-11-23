/************************************************************************
 * Copyright (C) 2021 Richard Palmer
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

#include <QTools/ProcessWrapper.h>
#include <cassert>


ProcessWrapper::Ptr ProcessWrapper::create( const QString &spath, const QStringList &args)
{
    return Ptr( new ProcessWrapper( spath, args));
}   // end create


ProcessWrapper::ProcessWrapper( const QString &spath, const QStringList &args)
{
    // Start the server as a separate detached process
    _process = new QProcess(this);
    _process->start( spath, args);
    _rdy = _process->waitForStarted();
    if ( _rdy)
        std::cerr << spath << " started as process " << _process->processId() << std::endl;
}   // end ctor


ProcessWrapper::~ProcessWrapper()
{
    _process->kill();
}   // end dtor


QString ProcessWrapper::query( const QString &msg)
{
    assert( _rdy);
    if ( !_rdy)
        return "";
    _process->write();

    _process->readLine();
}   // end query


void ProcessWrapper::_doOnErrorOccurred( QProcess::ProcessError err)
{
    _rdy = false;
}   // end _doOnErrorOccurred


