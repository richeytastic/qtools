#include "TwoHandleSlider.h"
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
    : QWidget(parent), _s0( new Slider), _s1( new Slider)
{
    QBoxLayout* layout = NULL;
    if ( orientVertically)
    {
        layout = new QVBoxLayout;
        _s0->qslider->setOrientation( Qt::Vertical);
        _s1->qslider->setOrientation( Qt::Vertical);
        //setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred);
    }   // end if
    else
    {
        layout = new QHBoxLayout;
        _s0->qslider->setOrientation( Qt::Horizontal);
        _s1->qslider->setOrientation( Qt::Horizontal);
        //setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed);
    }   // end else

    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    layout->addWidget( _s0->qslider);
    layout->addWidget( _s1->qslider);

    this->setLayout(layout);

    this->setStyleSheet( " \
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

    connect( _s0->qslider, SIGNAL( valueChanged(int)), this, SLOT( doOnValue0Changed()));
    connect( _s1->qslider, SIGNAL( valueChanged(int)), this, SLOT( doOnValue1Changed()));
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
// Keep the centrepoint between the slider handles given new desired values for the handles.
void TwoHandleSlider::updateSlidersMidpoint( float v0, float v1)
{
    disconnect( _s0->qslider, SIGNAL( valueChanged(int)), this, SLOT( doOnValue0Changed()));
    disconnect( _s1->qslider, SIGNAL( valueChanged(int)), this, SLOT( doOnValue1Changed()));

    const float mid = (v0 + v1)/2;
    _s0->max = _s1->min = mid;

    const float rng = _s1->max - _s0->min;
    const float lprop = (mid - _s0->min) / rng;
    const float rprop = 1.0 - lprop;

    const int totPxlWidth = width();
    const int lActWidth = roundf(lprop * totPxlWidth);
    const int rActWidth = totPxlWidth - lActWidth;
    const int h = this->height();
    _s0->qslider->resize( lActWidth, h);
    _s1->qslider->resize( rActWidth, h);
    _s1->qslider->move( lActWidth, 0);
    _s0->qslider->move( 0, 0);

    // Finally, set the actual positions of the sliders in accordance
    const double lIntValProp = double(v0 - _s0->min)/(mid - _s0->min);
    const double rIntValProp = double(v1 - mid)/(_s1->max - mid);
    _s0->qslider->setValue( int( lIntValProp * Slider::INT_RNG));
    _s1->qslider->setValue( int( rIntValProp * Slider::INT_RNG));

    connect( _s0->qslider, SIGNAL( valueChanged(int)), this, SLOT( doOnValue0Changed()));
    connect( _s1->qslider, SIGNAL( valueChanged(int)), this, SLOT( doOnValue1Changed()));
}   // end updateSlidersMidpoint


// public
void TwoHandleSlider::resetRange( float min, float max)
{
    _s0->min = min;
    _s1->max = max;
    reset();
}   // end resetRange


// public
void TwoHandleSlider::resetMinimum()
{
    updateSlidersMidpoint( _s0->min, value1());
}   // end resetMinimum


// public
void TwoHandleSlider::resetMaximum()
{
    updateSlidersMidpoint( value0(), _s1->max);
}   // end resetMaximum


// public
void TwoHandleSlider::reset()
{
    updateSlidersMidpoint( _s0->min, _s1->max);
}   // end reset


// public
void TwoHandleSlider::setPageStep( float pstep)
{
    _s0->qslider->setPageStep( _s0->toInt( pstep));
    _s1->qslider->setPageStep( _s1->toInt( -pstep));
}   // end setPageStep


// public
void TwoHandleSlider::setSingleStep( float sstep)
{
    _s0->qslider->setSingleStep( _s0->toInt( sstep));
    _s1->qslider->setSingleStep( _s1->toInt( -sstep));
}   // end setSingleStep


// public
float TwoHandleSlider::value0() const
{
    return _s0->value();
}   // end value0


// public
float TwoHandleSlider::value1() const
{
    return _s1->value();
}   // end value1


// public
void TwoHandleSlider::setValue0( float v)
{
    updateSlidersMidpoint( v, value1());
}   // end setValue0


// public
void TwoHandleSlider::setValue1( float v)
{
    updateSlidersMidpoint( value0(), v);
}   // end setValue1


// public
float TwoHandleSlider::minimum() const
{
    return _s0->min;
}   // end minimum


// public
float TwoHandleSlider::maximum() const
{
    return _s1->max;
}   // end maximum


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
