/************************************************************************
 * Copyright (C) 2020 Richard Palmer
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

#include <QTools/UpdateMeta.h>
using QTools::UpdateMeta;


UpdateMeta::UpdateMeta() : _major(0), _minor(0), _patch(0), _delete(false) {}


bool UpdateMeta::isValid() const
{
    return _major > 0 || _minor > 0 || _patch > 0;
}   // end isValid


bool UpdateMeta::operator<( const UpdateMeta &v) const
{
    const int mj = v.major();
    const int mn = v.minor();
    const int pt = v.patch();
    return (mj  > _major)
        || (mj == _major && mn  > _minor)
        || (mj == _major && mn == _minor && pt > _patch);
}   // end operator<


bool UpdateMeta::operator==( const UpdateMeta &v) const
{
    return v.major() == _major && v.minor() == _minor && v.patch() == _patch;
}   // end operator==

bool UpdateMeta::operator>=( const UpdateMeta &v) const { return !(*this < v);}

bool UpdateMeta::operator!=( const UpdateMeta &v) const { return !(*this == v);}

bool UpdateMeta::operator>( const UpdateMeta &v) const { return (*this >= v) && (*this != v);}

bool UpdateMeta::operator<=( const UpdateMeta &v) const { return !(*this > v);}
