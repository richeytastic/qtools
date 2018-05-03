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

#ifndef QTOOLS_COLOUR_MAPPING_WIDGET_H
#define QTOOLS_COLOUR_MAPPING_WIDGET_H

#include "QTools_Export.h"
#include <QWidget>

namespace Ui {
class ColourMappingWidget;
}

namespace QTools {
class TwoHandleSlider;

class ColourMappingWidget : public QWidget
{ Q_OBJECT
public:
    explicit ColourMappingWidget( QWidget *parent = 0);
    ~ColourMappingWidget();

    // Set/get whether displays three colour values in the widget
    // (min,mid,max), or just two (min,max).
    void setThreeBand( bool v);
    bool isThreeBand() const;

    // Set/get the colour mappings.
    void setMinColour( const QColor&);
    void setMidColour( const QColor&);
    void setMaxColour( const QColor&);
    const QColor& minColour() const { return _c0;}
    const QColor& midColour() const { return _c1;}
    const QColor& maxColour() const { return _c2;}

    // Set the scalar mapping range limits.
    void setScalarRangeLimits( float min, float max);

    // Set/get the current min and max scalar values from the slider.
    // When setting the min scalar, it must be lower than the current
    // max value. Similarly, when setting the max scalar, it must be
    // higher than the current min value.
    void setMinScalar(float);
    float minScalar() const;
    void setMaxScalar(float);
    float maxScalar() const;

    // Set/get # colour bands used to map colours between the selected min,max scalars (min 2).
    void setNumColours( size_t);
    size_t numColours() const;

signals:
    void onChanged();   // Emitted on changes to the widget

private slots:
    void doClickedSetMinColour();
    void doClickedSetMidColour();
    void doClickedSetMaxColour();

private:
    Ui::ColourMappingWidget *ui;
    TwoHandleSlider *_slider;
    QColor _c0, _c1, _c2;
};  // end class

}   // end namespace

#endif
