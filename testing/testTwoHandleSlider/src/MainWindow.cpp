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
