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

#ifndef QTOOLS_SCALAR_COLOUR_RANGE_MAPPER_H
#define QTOOLS_SCALAR_COLOUR_RANGE_MAPPER_H

/**
 * Store and manage scalar colours via lookup table.
 */

#include "QTools_Export.h"
#include <QColor>
#include <LookupTable.h>    // RVTK

namespace QTools {

class QTools_EXPORT ScalarColourRangeMapper
{
public:
    ScalarColourRangeMapper();

    // Set the min/max bounds of the mapping range.
    void setRangeLimits( float minv, float maxv);

    // Set the visible range (won't set outside of bounded range).
    void setVisibleRange( float smin, float smax);

    // Set the colours to be used for the current range.
    void setColours( const cv::Vec3b& c0, const cv::Vec3b& c1, const cv::Vec3b& c2);
    void setColours( const QColor& c0, const QColor& c1, const QColor& c2);
    void setMinColour( const QColor&);
    void setMidColour( const QColor&);
    void setMaxColour( const QColor&);

    void setNumColours( size_t);

    // Rebuild the lookup table: call after updating with fns setRangeLimits, setColours, setNumColours.
    void rebuild();

    RVTK::LookupTable& lookupTable() { return _ltable;}
    const RVTK::LookupTable& lookupTable() const { return _ltable;}

    // Get the range limits.
    const std::pair<float,float>& rangeLimits() const { return _rngl;}
    float minRange() const { return _rngl.first;}
    float maxRange() const { return _rngl.second;}

    // Get the visible limits.
    const std::pair<float,float>& visibleLimits() const { return _visl;}
    float minVisible() const { return _visl.first;}
    float maxVisible() const { return _visl.second;}

    // Get the colours being used for the mapping range.
    void colours( QColor& c0, QColor& c1, QColor& c2) const;
    QColor minColour() const;
    QColor midColour() const;
    QColor maxColour() const;

    // Get the number of colours being used.
    size_t numColours() const { return _ncols;}

private:
    size_t _ncols;                  // Number of colours
    std::pair<float,float> _rngl;   // Allowed range
    std::pair<float,float> _visl;   // Visible range
    cv::Vec3b _cols[3];             // min, mid, and max colours
    RVTK::LookupTable _ltable;      // The lookup table that does the actual mapping

    ScalarColourRangeMapper( const ScalarColourRangeMapper&) = delete;
    void operator=( const ScalarColourRangeMapper&) = delete;
};  // end class

}   // end namespace

#endif
