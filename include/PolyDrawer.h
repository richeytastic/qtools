#pragma once
#ifndef QTOOLS_POLY_DRAWER_H
#define QTOOLS_POLY_DRAWER_H

#include <QObject>
#include <QPolygonF>
#include <QPoint>
#include <QSize>
#include <QMouseEvent>
#include "QTools_Export.h"


namespace QTools
{

class QTools_EXPORT PolyDrawer : public QObject
{ Q_OBJECT
public:
    // Create a new drawing environment of given dimensions.
    // pxlDistToJoin: how close the last point must be (in city-block distance)
    // to the first point drawn when the mouse button is released to "join the points"
    // causing drawnPoly to be emitted. Default is a horizontally or vertically adjacent pixel.
    PolyDrawer( int width, int height, int pxlDistToJoin=1);
    ~PolyDrawer();

    void setPxlDistToJoin( int dist);

    void setDrawArea( int width, int height);
    void setDrawArea( const QSize&);
    const QSize& drawArea() const { return _area;}

signals:
    void drawnPoly( const QPolygonF&);   // When left mouse button is lifted to give the final polygon

    void pointAdded( const QPolygonF&); // Drawn another point (polygon so far)

    // After at least one emission of pointAdded if mouse button released without being close
    // enough to complete a joined polygon.
    void cancelDraw();

protected:
    virtual bool eventFilter( QObject*, QEvent*);   // Track mouse movements to draw polygons

private:
    bool doMousePressEvent( QMouseEvent*);
    bool doMouseReleaseEvent( QMouseEvent*);
    bool doMouseDoubleClickEvent( QMouseEvent*);
    bool doMouseMoveEvent( QMouseEvent*);
    bool doMouseLeaveEvent( QMouseEvent*);

    QSize _area;   // Drawing area
    int _joinPxlDist; // Max distance (city block) at which polygons are set
    QPolygonF _poly;
};  // end class

}   // end namespace


#endif
