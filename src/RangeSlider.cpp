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

#include <RangeSlider.h>
#include <algorithm>
#include <iostream>
#include <QHBoxLayout>
#include <QVBoxLayout>
using QTools::TwoHandleSlider;
using QTools::RangeSlider;

namespace {
QDoubleSpinBox* makeDoubleSpinBox()
{
    QDoubleSpinBox* sbox = new QDoubleSpinBox;
    sbox->setDecimals(2);
    sbox->setSingleStep(0.1);
    return sbox;
}   // end makeDoubleSpinBox
}   // end namespace


void RangeSlider::setHorizontalOrientation()
{
    if ( layout())
        delete layout();
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(_minSpin);
    layout->addWidget(_hslider);
    layout->addWidget(_maxSpin);
    setLayout(layout);

    _vslider->disconnect(this);
    connect( _hslider, &TwoHandleSlider::value0Changed, this, &RangeSlider::doOnSliderMinChanged);
    connect( _hslider, &TwoHandleSlider::value1Changed, this, &RangeSlider::doOnSliderMaxChanged);
}   // end setHorizontalOrientation


void RangeSlider::setVerticalOrientation()
{
    if ( layout())
        delete layout();
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(_maxSpin);
    layout->addWidget(_vslider);
    layout->addWidget(_minSpin);
    setLayout(layout);

    _hslider->disconnect(this);
    connect( _vslider, &TwoHandleSlider::value0Changed, this, &RangeSlider::doOnSliderMinChanged);
    connect( _vslider, &TwoHandleSlider::value1Changed, this, &RangeSlider::doOnSliderMaxChanged);
}   // end setVerticalOrientation


RangeSlider::RangeSlider( QWidget* parent)
    : QWidget(parent),
      _hslider( new TwoHandleSlider(nullptr, false)),
      _vslider( new TwoHandleSlider(nullptr, true)),
      _minSpin(makeDoubleSpinBox()),
      _maxSpin(makeDoubleSpinBox())
{
    // TODO make QTools::TwoHandleSlider to allow dynamic switching between horizontal and vertical orientations
    _minSpin->setAlignment(Qt::AlignRight);
    _maxSpin->setAlignment(Qt::AlignRight);
    setHorizontalOrientation();
    setRange(0.0f, 100.0f);
    setMin(0.0f);
    setMax(100.0f);
}   // end ctor


RangeSlider::~RangeSlider()
{
    delete _hslider;
    delete _vslider;
    delete _minSpin;
    delete _maxSpin;
}   // end dtor


void RangeSlider::setRange( float minv, float maxv)
{
    _hslider->resetRange( minv, maxv);
    _vslider->resetRange( minv, maxv);
    _minSpin->setRange( minv, maxv);
    _maxSpin->setRange( minv, maxv);
}   // end setRange


void RangeSlider::setMin( float v)
{
    _minSpin->disconnect(this);
    _hslider->setValue0( (float)v); // DOESN'T cause slider's value0Changed signal to fire
    _vslider->setValue0( (float)v); // DOESN'T cause slider's value0Changed signal to fire
    _minSpin->setValue(v);
    connect( _minSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &RangeSlider::doOnMinChanged);
}   // setMin


void RangeSlider::setMax( float v)
{
    _maxSpin->disconnect(this);
    _hslider->setValue1( (float)v); // DOESN'T cause slider's value1Changed signal to fire
    _vslider->setValue1( (float)v); // DOESN'T cause slider's value1Changed signal to fire
    _maxSpin->setValue(v);
    connect( _maxSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &RangeSlider::doOnMaxChanged);
}   // setMin


void RangeSlider::doOnSliderMinChanged( float v)
{
    _minSpin->setValue(v);
    emit rangeChanged( min(), max());
}   // doOnSliderMinChanged


void RangeSlider::doOnSliderMaxChanged( float v)
{
    _maxSpin->setValue(v);
    emit rangeChanged( min(), max());
}   // doOnSliderMaxChanged


void RangeSlider::doOnMinChanged( double v)
{
    setMin((float)v);
    emit rangeChanged( min(), max());
}   // end doOnMinChanged


void RangeSlider::doOnMaxChanged( double v)
{
    setMax((float)v);
    emit rangeChanged( min(), max());
}   // end doOnMaxChanged
