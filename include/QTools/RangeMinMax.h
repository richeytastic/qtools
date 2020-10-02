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

#ifndef QTOOLS_RANGE_MIN_MAX_H
#define QTOOLS_RANGE_MIN_MAX_H

/**
 * A composite widget meant for placing within a toolbar.
 * Initial range is set to [0.0f,100.0f] with 2 decimals
 * shown and a step size of 0.1f.
 */

#include <QTools_Export.h>
#include <QDoubleSpinBox>

namespace QTools {

class QTools_EXPORT RangeMinMax : public QWidget
{ Q_OBJECT
public:
    explicit RangeMinMax( QWidget* parent=nullptr);

    void setHorizontalOrientation();    // Default
    void setVerticalOrientation();

    void setNumDecimals(int);       // Default is 2
    void setSingleStepSize(float);  // Default is 0.1

    // Sets the allowed range for the min/max values.
    void setRange( float minv, float maxv);

    // Programmatically set the displayed min/max value without causing rangeChanged to fire.
    void setMin( float);
    void setMax( float);

    // Get the displayed min/max values.
    float min() const { return (float)_minSpin.value();}
    float max() const { return (float)_maxSpin.value();}

    // If set true, the min spin box is hidden and changes to the
    // min range value are negative the max range value unless
    void setMinHidden( bool);

signals:
    void valueChanged( float, float);   // Min or Max value changed

private slots:
    void _doOnMinChanged(double);
    void _doOnMaxChanged(double);

private:
    QDoubleSpinBox _minSpin;
    QDoubleSpinBox _maxSpin;
};  // end class

}   // end namespace

#endif
