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

#ifndef QTOOLS_TWO_HANDLE_SLIDER_H
#define QTOOLS_TWO_HANDLE_SLIDER_H

#include <QWidget>
#include "QTools_Export.h"

namespace QTools
{

class QTools_EXPORT TwoHandleSlider : public QWidget
{ Q_OBJECT
public:
    // Default range is [0,1] with page step of 0.1 and single step of 0.01
    TwoHandleSlider( QWidget *parent=NULL, bool orientVertically=false);
    virtual ~TwoHandleSlider();

    void resetRange( float min, float max);
    void reset();   // Reset handles to existing min and max

    void setPageStep( float pstep);
    void setSingleStep( float sstep);

    float value0() const;
    float value1() const;
    void setValue0( float);
    void setValue1( float);

    float minimum() const;
    float maximum() const;

signals:
    void value0Changed( float);
    void value1Changed( float);

private slots:
    void doOnValue0Changed();
    void doOnValue1Changed();

private:
    struct Slider;
    Slider *_s0, *_s1;
    bool _isVertical;
    void updateSlidersMidpoint( float, float);
    void connectSliders();
};  // end class


}   // end namespace

#endif
