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

#ifndef QTOOLS_PROCESS_WRAPPER_H
#define QTOOLS_PROCESS_WRAPPER_H

#include <QProcess>
#include <memory>

namespace QTools {

class QTools_EXPORT ProcessWrapper : QObject
{ Q_OBJECT
public:
    using Ptr = std::shared_ptr<ProcessWrapper>;

    // Creation of the process will block until started.
    static Ptr create( const QString &pathToProgram, const QStringList &programArgs);

    explicit ProcessWrapper( const QString &pathToProgram,
                             const QStringList &programArgs);
    ~ProcessWrapper() override;

    // Returns true if the process is running and ready to receive queries.
    inline bool isReady() const { return _rdy;}

    // Write the given string to the service and wait for a response (returned).
    QString query( const QString&);

private slots:
    void _doOnErrorOccurred( QProcess::ProcessError);

private:
    QProcess *_process;
    bool _rdy;
    ProcessWrapper( const ProcessWrapper&) = delete;
    void operator=( const ProcessWrapper&) = delete;
};  // end class

}   // end namespace

#endif
