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

#ifndef QTOOLS_FD_TOOL_PROCESS_H
#define QTOOLS_FD_TOOL_PROCESS_H

#include "QTools_Export.h"
#include <QString>
#include <QProcess>

namespace QTools {

class QTools_EXPORT FdToolProcess : public QProcess
{ Q_OBJECT
public:
    FdToolProcess();
    explicit FdToolProcess( const QString &fdToolPath);
    
    virtual ~FdToolProcess();

    // Returns true iff process was not already running and was successfully started.
    bool start( const QString &fdToolPath);
    bool start();

    // Returns true iff process was running and was successfully stopped.
    bool stop();

    bool isStarted() const;

    QString process( const QString &pathToImage);

private:
    QString _fdtoolPath;
    QProcess _fdtool;
    int _waitToKillMsecs;
    int _waitForStartMsecs;
    int _waitForImgReadMsecs;

    void _init();
};  // end class

}   // end namespace

#endif
