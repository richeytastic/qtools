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

#ifndef QTOOLS_RANGE_SLIDER_H
#define QTOOLS_RANGE_SLIDER_H

/**
 * A composite widget meant for placing within a toolbar.
 * Initial range is set to [0,100].
 */

#include "TwoHandleSlider.h"    // QTools
#include <QDoubleSpinBox>

namespace QTools {

class QTools_EXPORT RangeSlider : public QWidget
{ Q_OBJECT
public:
    RangeSlider( QWidget* parent=nullptr);
    ~RangeSlider() override;

    void setHorizontalOrientation();    // Default
    void setVerticalOrientation();

    // Programmatically set the min/max value without causing rangeChanged to fire.
    void setRange( float minv, float maxv);
    void setMin( float);
    void setMax( float);

    float min() const { return (float)_minSpin->value();}
    float max() const { return (float)_maxSpin->value();}

signals:
    void rangeChanged( float, float);   // Min or Max value changed

private slots:
    void doOnSliderMinChanged(float);
    void doOnSliderMaxChanged(float);
    void doOnMinChanged(double);
    void doOnMaxChanged(double);

private:
    QTools::TwoHandleSlider *_hslider;
    QTools::TwoHandleSlider *_vslider;
    QDoubleSpinBox *_minSpin;
    QDoubleSpinBox *_maxSpin;
};  // end class

}   // end namespace

#endif
