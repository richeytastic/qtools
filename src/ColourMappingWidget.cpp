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

#include <ColourMappingWidget.h>
#include <ui_ColourMappingWidget.h>
#include <TwoHandleSlider.h>
#include <QColorDialog>
#include <algorithm>
using QTools::ColourMappingWidget;
using QTools::TwoHandleSlider;


// public
ColourMappingWidget::ColourMappingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColourMappingWidget)
{
    ui->setupUi(this);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    connect( ui->numColoursSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ColourMappingWidget::onChanged);

    connect( ui->minColourButton, &QPushButton::clicked, this, &ColourMappingWidget::doClickedSetMinColour);
    connect( ui->midColourButton, &QPushButton::clicked, this, &ColourMappingWidget::doClickedSetMidColour);
    connect( ui->maxColourButton, &QPushButton::clicked, this, &ColourMappingWidget::doClickedSetMaxColour);

    _slider = new TwoHandleSlider(this,false);
    ui->colourRangeSliderLayout->addWidget( _slider);
    connect( _slider, &TwoHandleSlider::value0Changed, this, &ColourMappingWidget::onChanged);
    connect( _slider, &TwoHandleSlider::value1Changed, this, &ColourMappingWidget::onChanged);
}   // end ctor


ColourMappingWidget::~ColourMappingWidget() { delete ui;}


void ColourMappingWidget::setMinColour( const QColor& c)
{ ui->minColourButton->setStyleSheet(QString("background-color: %1;").arg(c.name())); _c0 = c;}

void ColourMappingWidget::setMidColour( const QColor& c)
{ ui->midColourButton->setStyleSheet(QString("background-color: %1;").arg(c.name())); _c1 = c;}

void ColourMappingWidget::setMaxColour( const QColor& c)
{ ui->maxColourButton->setStyleSheet(QString("background-color: %1;").arg(c.name())); _c2 = c;}


void ColourMappingWidget::setThreeBand( bool v) { ui->midColourButton->setVisible(v);}
bool ColourMappingWidget::isThreeBand() const { return ui->midColourButton->isVisible();}

size_t ColourMappingWidget::numColours() const { return ui->numColoursSpinBox->value();}

void ColourMappingWidget::setScalarRangeLimits( float minv, float maxv) { _slider->resetRange(minv,maxv);}

void ColourMappingWidget::setMinScalar( float v) { _slider->setValue0( std::min<float>(v, maxScalar()));}
void ColourMappingWidget::setMaxScalar( float v) { _slider->setValue1( std::max<float>(v, minScalar()));}
float ColourMappingWidget::minScalar() const { return _slider->value0();}
float ColourMappingWidget::maxScalar() const { return _slider->value1();}


void ColourMappingWidget::setNumColours( size_t nc)
{
    ui->numColoursSpinBox->setValue( (int)(std::max<size_t>(2, std::min<size_t>( 256, nc))));
}   // end setNumColours


// private slots
void ColourMappingWidget::doClickedSetMinColour()
{
    const QColor c = QColorDialog::getColor( _c0, this, "Choose Minimum Colour");
    if ( c.isValid())
    {
        setMinColour(c);
        emit onChanged();
    }   // end if
}   // end doClickedSetMinColour


void ColourMappingWidget::doClickedSetMidColour()
{
    const QColor c = QColorDialog::getColor( _c1, this, "Choose Middle Colour");
    if ( c.isValid())
    {
        setMidColour(c);
        emit onChanged();
    }   // end if
}   // end doClickedSetMidColour


void ColourMappingWidget::doClickedSetMaxColour()
{
    const QColor c = QColorDialog::getColor( _c2, this, "Choose Maximum Colour");
    if ( c.isValid())
    {
        setMaxColour(c);
        emit onChanged();
    }   // end if
}   // end doClickedSetMaxColour
