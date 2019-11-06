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

#include "QUtils.h"
#include <QStringRef>
#include <QDir>


QString QTools::getDirectory( const QString& fname, QString* rem)
{
    const int slashIdx = fname.lastIndexOf( QDir::separator());
    if ( rem != NULL)
    {
        const int dotIdx = fname.lastIndexOf('.');
        *rem = QStringRef(&fname, slashIdx+1, dotIdx-slashIdx-1).toString();
    }   // end if
    return QStringRef(&fname, 0, slashIdx).toString();
}   // end getDirectory
