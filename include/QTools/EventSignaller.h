/************************************************************************
 * Copyright (C) 2021 Richard Palmer
 *
 * Cliniface is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cliniface is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#ifndef QTOOLS_EVENT_SIGNALLER_H
#define QTOOLS_EVENT_SIGNALLER_H

#include "QTools_Export.h"
#include <QObject>
#include <QEvent>

namespace QTools {

class QTools_EXPORT EventSignaller : public QObject
{ Q_OBJECT
public:
    // Used to filter events of the given type. When an event of that type is
    // received on one of the installer objects, the onEvent signal is called
    // passing the object on which the event was received. If swallow is set
    // true, events of this type are swallowed and NOT propagated up the
    // inheritence hierarchy.
    EventSignaller( QEvent::Type, bool swallow=false);

    void install( QObject*);

signals:
    void onEvent( QObject*);

protected:
    bool eventFilter( QObject*, QEvent*) override;

private:
    const QEvent::Type _evType;
    const bool _doSwallow;
};  // end class

}   // end namespace

#endif
