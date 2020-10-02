/************************************************************************
 * Copyright (C) 2020 Richard Palmer
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

#include <RangeMinMax.h>
#include <algorithm>
#include <iostream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSignalBlocker>
using QTools::RangeMinMax;

void RangeMinMax::setHorizontalOrientation()
{
    if ( layout())
        delete layout();
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(&_minSpin);    // On left
    layout->addWidget(&_maxSpin);    // On right
    setLayout(layout);
}   // end setHorizontalOrientation

void RangeMinMax::setVerticalOrientation()
{
    if ( layout())
        delete layout();
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(&_maxSpin);    // On top
    layout->addWidget(&_minSpin);    // On bottom
    setLayout(layout);
}   // end setVerticalOrientation


RangeMinMax::RangeMinMax( QWidget* parent)
    : QWidget(parent)
{
    _minSpin.setButtonSymbols(QAbstractSpinBox::PlusMinus);
    _maxSpin.setButtonSymbols(QAbstractSpinBox::PlusMinus);
    _minSpin.setAlignment( Qt::AlignRight);
    _maxSpin.setAlignment( Qt::AlignRight);
    setHorizontalOrientation();
    setRange(0.0f, 100.0f);
    setMin(0.0f);
    setMax(100.0f);
    setNumDecimals(2);
    setSingleStepSize(0.1f);
    connect( &_minSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &RangeMinMax::_doOnMinChanged);
    connect( &_maxSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &RangeMinMax::_doOnMaxChanged);
}   // end ctor


void RangeMinMax::setNumDecimals( int nd)
{
    _minSpin.setDecimals(nd);
    _maxSpin.setDecimals(nd);
}   // end setNumDecimals


void RangeMinMax::setSingleStepSize( float s)
{
    _minSpin.setSingleStep(s);
    _maxSpin.setSingleStep(s);
}   // end setSingleStepSize


void RangeMinMax::setRange( float minv, float maxv)
{
    _minSpin.setRange( minv, maxv);
    _maxSpin.setRange( minv, maxv);
    _maxSpin.setMinimum( !_minSpin.isVisible() ? 0.0 : minv);
}   // end setRange


void RangeMinMax::setMin( float v)
{
    QSignalBlocker blocker(&_minSpin);
    _minSpin.setValue( v);
}   // setMin


void RangeMinMax::setMax( float v)
{
    QSignalBlocker blocker(&_maxSpin);
    _maxSpin.setValue( v);
}   // setMin


void RangeMinMax::setMinHidden( bool v)
{
    _minSpin.setVisible( !v);
    _maxSpin.setMinimum( v ? 0.0 : _minSpin.minimum());
}   // end setMinHidden


void RangeMinMax::_doOnMinChanged( double v)
{
    setMin((float)v);
    emit valueChanged( min(), max());
}   // end _doOnMinChanged


void RangeMinMax::_doOnMaxChanged( double v)
{
    setMax((float)v);
    const float maxVal = max();
    const float minVal = _minSpin.isVisible() ? min() : std::max( -maxVal, float(_minSpin.minimum()));
    emit valueChanged( minVal, maxVal);
}   // end _doOnMaxChanged
