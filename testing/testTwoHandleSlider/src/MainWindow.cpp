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

#include "MainWindow.h"
#include <iostream>

// public
MainWindow::MainWindow() : _slider( new QTools::TwoHandleSlider(this))
{
    setCentralWidget( _slider);
    _slider->resetRange(-10,10);
    connect( _slider, SIGNAL( value0Changed(float)), this, SLOT( doOnValuesChanged()));
    connect( _slider, SIGNAL( value1Changed(float)), this, SLOT( doOnValuesChanged()));
}   // end ctor

// private slot
void MainWindow::doOnValuesChanged()
{
    std::cerr << "\r[" << _slider->value0()
         << "] ---- [" << _slider->value1() << "]                         ";
}   // end doOnValuesChanged
