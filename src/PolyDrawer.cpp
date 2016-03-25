#include "PolyDrawer.h"
using QTools::PolyDrawer;
#include <algorithm>
#include <cstdlib>  // abs


PolyDrawer::PolyDrawer( int width, int height, int dist) : _area(width, height), _joinPxlDist(dist)
{}   // end ctor


PolyDrawer::~PolyDrawer()
{}   // end dtor


void PolyDrawer::setDrawArea( int width, int height)
{
    _area.setWidth( width);
    _area.setHeight( height);
}   // end setDrawArea


void PolyDrawer::setDrawArea( const QSize& qsz)
{
    _area = qsz;
}   // end setDrawArea


void PolyDrawer::setPxlDistToJoin( int dist)
{
    _joinPxlDist = std::max<int>( dist, 1);
}   // end setPxlDistToJoin


// virtual
bool PolyDrawer::eventFilter( QObject*, QEvent* evt)
{
    if ( evt->type() == QEvent::MouseMove)
        return doMouseMoveEvent( dynamic_cast<QMouseEvent*>(evt));
    else if ( evt->type() == QEvent::MouseButtonPress)
        return doMousePressEvent( dynamic_cast<QMouseEvent*>(evt));
    else if ( evt->type() == QEvent::MouseButtonRelease)
        return doMouseReleaseEvent( dynamic_cast<QMouseEvent*>(evt));
    else if ( evt->type() == QEvent::MouseButtonDblClick)
        return doMouseDoubleClickEvent( dynamic_cast<QMouseEvent*>(evt));
    else if ( evt->type() == QEvent::Leave)
        return doMouseLeaveEvent( dynamic_cast<QMouseEvent*>(evt));
    return false;
}   // end eventFilter


QPoint getCoords( QMouseEvent* evt)
{
    return QPoint( evt->pos().x(), evt->pos().y());
}   // end getCoords


// Check if points a and b are within dist (city block) of one another
bool isWithinJoinDist( const QPoint& a, const QPoint&b, int dist)
{
    return abs(a.x() - b.x()) + abs(a.y() - b.y()) <= dist;
}   // end isWithinJoinDist


// private
bool PolyDrawer::doMousePressEvent( QMouseEvent* evt)
{
    bool swallowed = false;
    if ( evt->button() == Qt::LeftButton)   // Right mouse button ignored
    {
        swallowed = true;
        _poly.clear();
        QPoint p = getCoords(evt);
        _poly.push_back( p);
        emit pointAdded( _poly);
    }   // end if

    return swallowed;
}   // end doMousePressEvent



// private
bool PolyDrawer::doMouseReleaseEvent( QMouseEvent* evt)
{
    bool swallowed = false;
    if ( evt->button() == Qt::LeftButton)   // Right mouse button ignored
    {
        swallowed = true;
        if ( !_poly.empty())
        {
            const QPoint p = getCoords(evt);
            const QPoint p0( _poly[0].x() * _area.width(), _poly[0].y() * _area.height());
            if ( isWithinJoinDist( p, p0, _joinPxlDist))
                emit drawnPoly( _poly);
            else
                emit cancelDraw();
            _poly.clear();
        }   // end if
    }   // end if

    return swallowed;
}   // end doMouseReleaseEvent



// private
bool PolyDrawer::doMouseDoubleClickEvent( QMouseEvent* evt)
{
    return false;
}   // end doMouseDoubleClickEvent



// private
bool PolyDrawer::doMouseMoveEvent( QMouseEvent* evt)
{
    bool swallowed = false;
    if ( !_poly.empty())   // Right mouse button ignored
    {
        swallowed = true;
        QPoint p = getCoords(evt);
        // Now set as a scaled value
        const QPointF pf( float(p.x()) / _area.width(), float(p.y()) / _area.height());
        _poly << pf;
        emit pointAdded( _poly);
    }   // end if

    return swallowed;
}   // end doMouseMoveEvent



// private
bool PolyDrawer::doMouseLeaveEvent( QMouseEvent* evt)
{
    bool swallowed = false;
    if ( !_poly.empty())   // Right mouse button ignored
    {
        swallowed = true;
        QPoint p = getCoords(evt);
        // Bound x
        p.setX( std::min<int>( p.x(), _area.width()-1));
        p.setX( std::max<int>( p.x(), 0));
        // Bound y
        p.setY( std::min<int>( p.y(), _area.height()-1));
        p.setY( std::max<int>( p.y(), 0));

        // Now set as a scaled value
        const QPointF pf( float(p.x()) / _area.width(), float(p.y()) / _area.height());
        _poly << pf;
        emit pointAdded( _poly);
    }   // end if

    return swallowed;
}   // end doMouseLeaveEvent
