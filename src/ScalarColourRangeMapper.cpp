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

#include <ScalarColourRangeMapper.h>
using QTools::ScalarColourRangeMapper;


ScalarColourRangeMapper::ScalarColourRangeMapper()
    : _ncols(100), _rngl(0.0f,1.0f), _visl(0.0f,1.0f)
{
    _cols[0] = cv::Vec3b(0,  0,  255);  // Red (BGR)
    _cols[1] = cv::Vec3b(255,255,255);  // White (BGR)
    _cols[2] = cv::Vec3b(255,  0,  0);  // Blue (BGR)
    rebuild();
}   // end ctor


void ScalarColourRangeMapper::setRangeLimits( float rmin, float rmax)
{
    assert(rmin <= rmax);
    _rngl.first = rmin;
    _rngl.second = rmax;
    setVisibleLimits( _visl.first, _visl.second);
}   // end setRangeLimits


void ScalarColourRangeMapper::setVisibleLimits( float vmin, float vmax)
{
    // If the visible range sits outside of the range limits, set to the range limits.
    const float rmin = _rngl.first;
    const float rmax = _rngl.second;

    if ( vmin < rmin || vmin > rmax)
        vmin = rmin;
    if ( vmax < rmin || vmax > rmax)
        vmax = rmax;

    assert(vmin <= vmax);

    _visl.first = vmin;
    _visl.second = vmax;
}   // end setVisibleLimits


void ScalarColourRangeMapper::setNumColours( size_t nc) { _ncols = std::max<size_t>(2, nc);}

void ScalarColourRangeMapper::setColours( const cv::Vec3b& c0, const cv::Vec3b& c1, const cv::Vec3b& c2)
{
    _cols[0] = c0;
    _cols[1] = c1;
    _cols[2] = c2;
}   // end setColours


void ScalarColourRangeMapper::setColours( const QColor& c0, const QColor& c1, const QColor& c2)
{
    const cv::Vec3b minCol( c0.red(), c0.green(), c0.blue());
    const cv::Vec3b midCol( c1.red(), c1.green(), c1.blue());
    const cv::Vec3b maxCol( c2.red(), c2.green(), c2.blue());
    setColours( minCol, midCol, maxCol);
}   // end setColours

void ScalarColourRangeMapper::setMinColour( const QColor& c) { _cols[0] = cv::Vec3b( c.red(), c.green(), c.blue());}
void ScalarColourRangeMapper::setMidColour( const QColor& c) { _cols[1] = cv::Vec3b( c.red(), c.green(), c.blue());}
void ScalarColourRangeMapper::setMaxColour( const QColor& c) { _cols[2] = cv::Vec3b( c.red(), c.green(), c.blue());}

void ScalarColourRangeMapper::colours( QColor& c0, QColor& c1, QColor& c2) const
{
    c0 = minColour();
    c1 = midColour();
    c2 = maxColour();
}   // end colours

QColor ScalarColourRangeMapper::minColour() const { return QColor( _cols[0][0], _cols[0][1], _cols[0][2]);}
QColor ScalarColourRangeMapper::midColour() const { return QColor( _cols[1][0], _cols[1][1], _cols[1][2]);}
QColor ScalarColourRangeMapper::maxColour() const { return QColor( _cols[2][0], _cols[2][1], _cols[2][2]);}


void ScalarColourRangeMapper::rebuild()
{
    const float minv = _visl.first;
    const float maxv = _visl.second;

    size_t nc0 = _ncols;
    size_t nc1 = _ncols;
    if ( maxv <= 0.0f)
        nc1 = 0;    // Will use upper (positive) colour range only
    else if ( minv >= 0.0f)
        nc0 = 0;    // Will use lower (negative) colour range only
    else
    {   // Calculate the proportional number of colours below zero (nc0) and above zero (nc1)
        nc0 = (size_t)(double(_ncols) * fabs(minv)/double(maxv - minv) + 0.5);
        nc1 = _ncols - nc0;
    }   // end else

    if ( nc1 == 0)
        _ltable.setColours( _cols[0], _cols[1], nc0);
    else if ( nc0 == 0)
        _ltable.setColours( _cols[1], _cols[2], nc1);
    else
        _ltable.setColours( _cols[0], _cols[1], _cols[2], nc0, nc1);
}   // end rebuild

