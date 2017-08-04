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

#pragma once
#ifndef QTOOLS_VIEWNAVIGATOR_H
#define QTOOLS_VIEWNAVIGATOR_H

#include "QTools_Export.h"
#include <QWidget>
#include <string>
using std::string;


namespace Ui {
class ViewNavigator;
}

namespace QTools
{

class QTools_EXPORT ViewNavigator : public QWidget
{ Q_OBJECT
public:
    enum ViewFace
    {
        NONE = -1,
        FRONT = 0,
        LEFT = 1,
        REAR = 2,
        RIGHT = 3
    };  // end enum

    enum ViewType
    {
        IMAGE,
        DEPTH,
        POINTS
    };  // end enum

    explicit ViewNavigator(QWidget *parent = 0);
    ~ViewNavigator();

    static string viewFaceToString( const ViewFace&);

    ViewFace getViewDirection() const;   // Current user set view direction
    void setViewDirection( ViewFace);   // Set the current view direction (sets UI elements)

    ViewType getViewType() const;   // Get whether IMAGE, DEPTH or POINTS
    void setViewType( ViewType);  // Set the current view type (sets UI elements)

    void enableControls( bool);  // Enable/disable all controls other than view face selectors
    void reset();

signals:
    void selectedView( ViewNavigator::ViewFace);   // Emitted when user selects a view to display

    void selectedImage();   // Emitted if user wants to view the image
    void selectedDepth();   // Emitted if user wants to view the depth image
    void selectedPoints();  // Emitted if user wants to view the points

private slots:
    void userSelectViewDirectionSlot(); // Select a view direction to display

private:
    Ui::ViewNavigator *ui;
};  // end class

}   // end namespace

#endif
