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

#ifndef QTOOLS_QUTILS_H
#define QTOOLS_QUTILS_H

#include "QTools_Export.h"
#include <QString>

namespace QTools {

// Get the directory part of fname and place the remainder without file
// extension (if any) in rem (if not null).
QTools_EXPORT QString getDirectory( const QString &fname, QString* rem=nullptr);

}   // end namespace

#endif
