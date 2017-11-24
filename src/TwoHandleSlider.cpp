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

#include <TwoHandleSlider.h>
using QTools::TwoHandleSlider;
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>


struct TwoHandleSlider::Slider
{
    static const int INT_RNG = 2147483647; // Full positive range of signed int for accuracy

    Slider() : qslider(new QSlider), min(0.0f), max(0.0f)
    {
        qslider->setRange(0, INT_RNG);
    }   // end ctor

    ~Slider()
    {
        delete qslider;
    }   // end dtor

    QSlider* qslider;
    float min, max;

    float toFloat( int v) const
    {
        float prop = std::max<float>( -1.0f,
                     std::min<float>( 1.0f, float(v) / (qslider->maximum() - qslider->minimum())));
        return prop * (max - min);
    }   // end toFloat

    int toInt( float v) const
    {
        float prop = std::max<float>( -1.0f, std::min<float>( 1.0f, v / (max - min)));
        return (int)(prop * (qslider->maximum() - qslider->minimum()));
    }   // end toInt

    float value() const
    {
        return min + toFloat( qslider->value());
    }   // end value
};  // end struct


TwoHandleSlider::TwoHandleSlider( QWidget* parent, bool orientVertically)
    : QWidget(parent), _s0( new Slider), _s1( new Slider), _isVertical(orientVertically)
{
    QBoxLayout* layout = NULL;
    if ( orientVertically)
    {
        layout = new QVBoxLayout;
        _s0->qslider->setOrientation( Qt::Vertical);
        _s1->qslider->setOrientation( Qt::Vertical);
        //setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred);
        layout->addWidget( _s0->qslider);   // Top - min is middle
        layout->addWidget( _s1->qslider);   // Bottom - max is middle
    }   // end if
    else
    {
        layout = new QHBoxLayout;
        _s0->qslider->setOrientation( Qt::Horizontal);
        _s1->qslider->setOrientation( Qt::Horizontal);
        //setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed);
        layout->addWidget( _s0->qslider);   // Left - max is middle
        layout->addWidget( _s1->qslider);   // Right - min is middle
    }   // end else

    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    setLayout(layout);

    if ( orientVertically)
    {
        setStyleSheet( " \
            QSlider::groove:vertical { \
            border: 1px solid #999999; \
            width: 8px; /* groove expands to size of slider by def. giving it width, it has fixed size */ \
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4); \
            margin: 2px 0; \
            } \
            \
            QSlider::handle:vertical { \
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); \
            border: 1px solid #5c5c5c; \
            height: 18px; \
            margin: 0 -2px; /* handle placed by def on contents rect of groove. Expand outside the groove */ \
            border-radius: 3px; \
            }"
            );
    }   // end if
    else
    {
        setStyleSheet( " \
            QSlider::groove:horizontal { \
            border: 1px solid #999999; \
            height: 8px; /* groove expands to size of slider by def. giving it height, it has fixed size */ \
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4); \
            margin: 2px 0; \
            } \
            \
            QSlider::handle:horizontal { \
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f); \
            border: 1px solid #5c5c5c; \
            width: 18px; \
            margin: -2px 0; /* handle placed by def on contents rect of groove. Expand outside the groove */ \
            border-radius: 3px; \
            }"
            );
    }   // end else

    connectSliders();
    setPageStep( 0.1f);
    setSingleStep( 0.01f);

    resetRange( 0, 1);
}   // end ctor


TwoHandleSlider::~TwoHandleSlider()
{
    delete _s0;
    delete _s1;
}   // end dtor


// private
void TwoHandleSlider::connectSliders()
{
    if ( _isVertical)
    {
        connect( _s1->qslider, &QSlider::valueChanged, this, &TwoHandleSlider::doOnValue0Changed);
        connect( _s0->qslider, &QSlider::valueChanged, this, &TwoHandleSlider::doOnValue1Changed);
    }   // end if
    else
    {
        connect( _s0->qslider, &QSlider::valueChanged, this, &TwoHandleSlider::doOnValue0Changed);
        connect( _s1->qslider, &QSlider::valueChanged, this, &TwoHandleSlider::doOnValue1Changed);
    }   // end else
}   // end connectSliders

// For vertical:
// _s0->qslider   // Top - min is middle
// _s1->qslider   // Bottom - max is middle

// private
// Keep the centrepoint between the slider handles given new desired values for the handles.
void TwoHandleSlider::updateSlidersMidpoint( float v0, float v1)
{
    _s0->qslider->disconnect(this);
    _s1->qslider->disconnect(this);

    const float mid = (v0 + v1)/2;
    _s0->max = _s1->min = mid;
    if ( _isVertical)
        _s0->min = _s1->max = mid;

    const float rng = maximum() - minimum();
    const float lprop = (mid - minimum()) / rng;
    //const float rprop = 1.0 - lprop;

    const int totPxl = _isVertical ? height() : width();
    const int lPxl = roundf(lprop * totPxl);    // Left / bottom (low section)
    const int hPxl = totPxl - lPxl;             // Right / top (high section)
    const int dPxl = _isVertical ? this->width() : this->height(); // Fixed dimension not moving

    if ( _isVertical)
    {
        _s0->qslider->resize( dPxl, hPxl);
        _s1->qslider->resize( dPxl, lPxl);
        _s0->qslider->move( 0, 0);
        _s1->qslider->move( 0, hPxl);
    }   // end if
    else
    {
        _s0->qslider->resize( lPxl, dPxl);
        _s1->qslider->resize( hPxl, dPxl);
        _s0->qslider->move( 0, 0);
        _s1->qslider->move( lPxl, 0);
    }   // end else

    // Finally, set the actual positions of the sliders in accordance
    const double lIntValProp = double(v0 - minimum())/(mid - minimum());
    const double rIntValProp = double(v1 - mid)/(maximum() - mid);

    if ( _isVertical)
    {
        _s1->qslider->setValue( int( lIntValProp * Slider::INT_RNG));
        _s0->qslider->setValue( int( rIntValProp * Slider::INT_RNG));
    }   // end if
    else
    {
        _s0->qslider->setValue( int( lIntValProp * Slider::INT_RNG));
        _s1->qslider->setValue( int( rIntValProp * Slider::INT_RNG));
    }   // end else

    connectSliders();
}   // end updateSlidersMidpoint


// public
void TwoHandleSlider::resetRange( float min, float max)
{
    if ( _isVertical)
    {
        _s0->max = max;
        _s1->min = min;
    }   // end if
    else
    {
        _s0->min = min;
        _s1->max = max;
    }   // end else
    reset();
}   // end resetRange


// public
void TwoHandleSlider::reset() { updateSlidersMidpoint( minimum(), maximum());}
float TwoHandleSlider::minimum() const { return _isVertical ? _s1->min : _s0->min;}
float TwoHandleSlider::maximum() const { return _isVertical ? _s0->max : _s1->max;}
float TwoHandleSlider::value0() const { return _isVertical ? _s1->value() : _s0->value();}  // Lower value
float TwoHandleSlider::value1() const { return _isVertical ? _s0->value() : _s1->value();}  // Higher value
void TwoHandleSlider::setValue0( float v) { updateSlidersMidpoint( v, value1());}
void TwoHandleSlider::setValue1( float v) { updateSlidersMidpoint( value0(), v);}


// public
void TwoHandleSlider::setPageStep( float pstep)
{
    if ( _isVertical)
    {   // Handles coming together on vertical
        _s0->qslider->setPageStep( _s0->toInt( -pstep));
        _s1->qslider->setPageStep( _s1->toInt( pstep));
    }   // end if
    else
    {   // Handles coming together on horizontal
        _s0->qslider->setPageStep( _s0->toInt( pstep));
        _s1->qslider->setPageStep( _s1->toInt( -pstep));
    }   // end else
}   // end setPageStep


// public
void TwoHandleSlider::setSingleStep( float sstep)
{
    if ( _isVertical)
    {
        _s0->qslider->setSingleStep( _s0->toInt( -sstep));
        _s1->qslider->setSingleStep( _s1->toInt( sstep));
    }   // end if
    else
    {
        _s0->qslider->setSingleStep( _s0->toInt( sstep));
        _s1->qslider->setSingleStep( _s1->toInt( -sstep));
    }   // end else
}   // end setSingleStep


// private slot
void TwoHandleSlider::doOnValue0Changed()
{
    updateSlidersMidpoint( value0(), value1());
    emit value0Changed( value0());
}   // end doOnValue0Changed


// private slot
void TwoHandleSlider::doOnValue1Changed()
{
    updateSlidersMidpoint( value0(), value1());
    emit value1Changed( value1());
}   // end doOnValue1Changed
