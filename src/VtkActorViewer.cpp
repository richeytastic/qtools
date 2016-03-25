#include "VtkActorViewer.h"
using QTools::VtkActorViewer;
#include <cassert>
#include <iostream>


VtkActorViewer::VtkActorViewer( QWidget *parent) : QVTKWidget( parent),
    _viewer( RVTK::Viewer::create()), _autoUpdateRender(false)
{
    //QWidget::setWindowFlags(Qt::Window);
    vtkSmartPointer<vtkRenderWindow> renderWindow = _viewer->getRenderWindow();
    this->SetRenderWindow( renderWindow);
    _viewer->getCamera( _camPos, _camFocus, _camUp); // Set initial camera parameters
}	// end ctor



VtkActorViewer::~VtkActorViewer()
{
    //_connections->Delete();
}	// end dtor


// public
void VtkActorViewer::setInteractor( vtkSmartPointer<vtkInteractorStyle> intStyle)
{
    _viewer->getRenderWindow()->GetInteractor()->SetInteractorStyle( intStyle);
    intStyle->GetInteractor()->SetRenderWindow( _viewer->getRenderWindow());
}   // end setInteractor


// public
void VtkActorViewer::setSize( int w, int h)
{
    this->resize(w,h);
    this->setMinimumSize(w,h);
    this->setMaximumSize(w,h);
}   // end setSize


// public
void VtkActorViewer::addActor( vtkActor* obj)
{
    _viewer->addActor( obj);
    if ( _autoUpdateRender)
        updateRender();
}   // end addActor


// public
void VtkActorViewer::removeActor( vtkActor* obj)
{
    _viewer->removeActor( obj);
    if ( _autoUpdateRender)
        updateRender();
}   // end removeActor


// public
void VtkActorViewer::clear()
{
    _viewer->clear();
    if ( _autoUpdateRender)
        updateRender();
}   // end clear


// public
void VtkActorViewer::adjustCamera( const cv::Vec3f &pos, const cv::Vec3f &foc, const cv::Vec3f &up)
{
    _viewer->resetCamera( pos, foc, up, 30);
    if ( _autoUpdateRender)
        updateRender();
}   // end adjustCamera


// public
void VtkActorViewer::setCameraFocus( const cv::Vec3f& focus)
{
    _viewer->setCameraFocus( focus);
    if ( _autoUpdateRender)
        updateRender();
}   // end setCameraFocus


// public
void VtkActorViewer::setCameraPosition( const cv::Vec3f& pos)
{
    _viewer->setCameraPosition( pos);
    if ( _autoUpdateRender)
        updateRender();
}   // end setCameraPosition


// public
void VtkActorViewer::setCameraViewUp( const cv::Vec3f& up)
{
    _viewer->setCameraViewUp( up);
    if ( _autoUpdateRender)
        updateRender();
}   // end setCameraViewUp


// public
cv::Mat_<cv::Vec3b> VtkActorViewer::grabImage() const
{
    return _viewer->extractImage();
}   // end grabImage


// public
void VtkActorViewer::setBackgroundWhite( bool on)
{
    _viewer->changeBackground( on ? 255 : 0);
    if ( _autoUpdateRender)
        updateRender();
}	// end setBackgroundWhite


// public
void VtkActorViewer::setStereoRendering( bool on)
{
    _viewer->setStereoRendering( on);
    if ( _autoUpdateRender)
        updateRender();
}	// end setStereoRendering



// public
void VtkActorViewer::setOrthogonal( bool on)
{
    _viewer->setPerspective( !on);
    if ( on)
        _viewer->setParallelScale( 10.0);    // For parallel projection
    else
        _viewer->setFieldOfView( 90.0);  // For perspective projection

    if ( _autoUpdateRender)
        updateRender();
}	// end setPerspective



// public
void VtkActorViewer::setResetCamera( const cv::Vec3f &pos, const cv::Vec3f &foc, const cv::Vec3f &up)
{
    _camPos = pos;
    _camFocus = foc;
    _camUp = up;
    resetCamera();
}   // end setResetCamera



// public
void VtkActorViewer::setLighting( const vector<cv::Vec3f>& lpos, const vector<cv::Vec3f>& lfoc, bool headlight)
{
    const int numLights = (int)lpos.size();
    assert( numLights == lfoc.size());
    double* intensities = (double*)cv::fastMalloc(numLights * sizeof(double));
    for ( int i = 0; i < numLights; ++i)
        intensities[i] = 1;
    _viewer->resetSceneLights( numLights, &lpos[0], &lfoc[0], intensities);
    _viewer->setHeadlightEnabled( headlight);
    cv::fastFree(intensities);
    if ( _autoUpdateRender)
        updateRender();
}   // end setLighting


// public
void VtkActorViewer::setHeadlight( bool enabled)
{
    _viewer->setHeadlightEnabled( enabled);
    if ( _autoUpdateRender)
        updateRender();
}   // end setHeadlight



// public
void VtkActorViewer::resetCamera()
{
    adjustCamera( _camPos, _camFocus, _camUp);
}   // end resetCamera



// public
void VtkActorViewer::printCameraParams() const
{
    _viewer->printCameraDetails( std::cerr);
}   // end printCameraParams


// public
vtkSmartPointer<vtkActor> VtkActorViewer::pickActor( const cv::Point& pt, const vector<vtkSmartPointer<vtkActor> >& pactors) const
{
    cv::Point p( pt.x, size().height() - pt.y - 1); // Make point using the BOTTOM LEFT as origin for VTK render window
    return _viewer->pickActor(p, pactors);
}   // end pickActor


// public
vtkSmartPointer<vtkActor> VtkActorViewer::pickActor( const cv::Point& pt) const
{
    cv::Point p( pt.x, size().height() - pt.y - 1); // Make point using the BOTTOM LEFT as origin for VTK render window
    return _viewer->pickActor(p);
}   // end pickActor



// public
int VtkActorViewer::pickCell(const cv::Point &topLeftPoint) const
{
    cv::Point p( topLeftPoint.x, size().height() - topLeftPoint.y - 1); // Make point using the BOTTOM LEFT as origin for VTK render window
    return _viewer->pickCell(p);
}   // end pickCell



// public
int VtkActorViewer::pickActorCells( const vector<cv::Point>& points, const vtkActor* actor, vector<int>& cellIds) const
{
    if ( actor == NULL)
        return 0;

    // Convert points to use BOTTOM LEFT as origin for VTK.
    const int numPoints = (int)points.size();
    const int winHeight = size().height() - 1;
    vector<cv::Point> bottomLeftPoints( numPoints);
    for ( int i = 0; i < numPoints; ++i)
    {
        const cv::Point& p = points[i];
        bottomLeftPoints[i] = cv::Point(p.x, winHeight - p.y);
    }   // end for

    return _viewer->pickActorCells( bottomLeftPoints, actor, cellIds);
}   // end pickActorCells


// public
int VtkActorViewer::pickActorCells( const cv::Mat& inmask, const vtkActor* actor, vector<int>& cellIds) const
{
    if ( actor == NULL)
        return 0;

    assert( inmask.rows == size().height() && inmask.cols == size().width());
    assert( inmask.channels() == 1);

    typedef unsigned char byte;
    cv::Mat_<byte> mask;
    inmask.convertTo( mask, CV_8U);

    // Convert points to use BOTTOM LEFT as origin for VTK.
    const int winHeight = size().height() - 1;
    vector<cv::Point> bottomLeftPoints;

    for ( int i = 0; i < mask.rows; ++i)
    {
        const byte* maskrow = mask.ptr<byte>(i);
        for ( int j = 0; j < mask.cols; ++j)
        {
            if ( maskrow[j])
                bottomLeftPoints.push_back( cv::Point(j, winHeight - i));
        }   // end for - cols
    }   // end for - rows

    return _viewer->pickActorCells( bottomLeftPoints, actor, cellIds);
}   // end pickActorCells


// public
cv::Vec3f VtkActorViewer::pickWorldPosition( const cv::Point& p) const
{
    cv::Point np( p.x, size().height() - p.y - 1);  // Reverse y
    vtkSmartPointer<vtkWorldPointPicker> picker = vtkSmartPointer<vtkWorldPointPicker>::New();  // Hardware accelerated
    picker->Pick( np.x, np.y, 0, _viewer->getRenderer());
    const double* worldPos = picker->GetPickPosition();
    return cv::Vec3f( worldPos[0], worldPos[1], worldPos[2]);
}   // end pickWorldPosition


// public
cv::Vec3f VtkActorViewer::pickWorldPosition( const cv::Point2f& p) const
{
    cv::Point p1( (int)cvRound(p.x * (size().width()-1)), (int)cvRound(p.y * (size().height()-1)));
    return pickWorldPosition( p1);
}   // end pickWorldPosition


// public
cv::Vec3f VtkActorViewer::pickWorldMeanPosition( const cv::Point& p) const
{
    cv::Point np( p.x, size().height() - p.y - 1);  // Reverse y
    vtkSmartPointer<vtkWorldPointPicker> picker = vtkSmartPointer<vtkWorldPointPicker>::New();  // Hardware accelerated

    // Cross of points to get 3D locations for
    const int NPT = 5;
    std::vector<cv::Point> pts(NPT);
    pts[0] = np;
    pts[1] = np;
    pts[2] = np;
    pts[3] = np;
    pts[4] = np;
    pts[1].x--;
    pts[2].x++;
    pts[3].y--;
    pts[4].y++;

    cv::Vec3f mv(0,0,0);   // Will be mean vector position
    std::vector<cv::Vec3f> vs(NPT);
    const vtkSmartPointer<vtkRenderer> ren = _viewer->getRenderer();
    for ( int i = 0; i < NPT; ++i)
    {
        picker->Pick( pts[i].x, pts[i].y, 0, ren);
        const double* worldPos = picker->GetPickPosition();
        vs[i] = cv::Vec3f( worldPos[0], worldPos[1], worldPos[2]);
        mv += vs[i];
    }   // end for
    mv /= NPT;

    std::vector<cv::Vec3f> vd(NPT);   // Vector position absolute differences
    cv::Vec3f stddev(0,0,0);   // Std-dev in three dimensions
    for ( int i = 0; i < NPT; ++i)
    {
        vd[i] = cv::Vec3f( fabsf( vs[i][0] - mv[0]), fabsf( vs[i][1] - mv[1]), fabsf( vs[i][2] - mv[2]));
        stddev += cv::Vec3f( powf( vd[i][0], 2), powf( vd[i][1], 2), powf( vd[i][2], 2));
    }   // end for

    // Only use values < 2 std-dev away in any axis
    stddev = 2*cv::Vec3f( sqrtf(stddev[0]/NPT), sqrtf(stddev[1]/NPT), sqrtf(stddev[2]/NPT));
    int usedCount = 0;
    cv::Vec3f fpos(0,0,0);
    for ( int i = 0; i < NPT; ++i)
    {
        const cv::Vec3f& v = vd[i];
        if ( v[0] < stddev[0] && v[1] < stddev[1] && v[2] < stddev[2])
        {
            fpos += vs[i];
            usedCount++;
        }   // end if
    }   // end for

    if ( !usedCount)
        fpos = vs[0];
    else
        fpos /= usedCount;

    return fpos;
}   // end pickWorldMeanPosition


// public
cv::Vec3f VtkActorViewer::pickNormal( const cv::Point& p) const
{
    cv::Point np( p.x, size().height() - p.y - 1);  // Reverse y
    vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
    if ( !picker->Pick( np.x, np.y, 0, _viewer->getRenderer()))
        return cv::Vec3f(0,0,0);
    const double* normal = picker->GetPickNormal();
    return cv::Vec3f( normal[0], normal[1], normal[2]);
}   // end pickNormal


// public
cv::Point VtkActorViewer::projectToDisplay( const cv::Vec3f& v) const
{
    vtkSmartPointer<vtkCoordinate> coordConverter = vtkSmartPointer<vtkCoordinate>::New();
    coordConverter->SetCoordinateSystemToWorld();
    coordConverter->SetValue( v[0], v[1], v[2]);
    const int* dpos = coordConverter->GetComputedDisplayValue( _viewer->getRenderer());
    return cv::Point( dpos[0], this->size().height() - dpos[1] - 1);
}   // end projectToDisplay


// public
cv::Point2f VtkActorViewer::projectToDisplayProportion( const cv::Vec3f& v) const
{
    const cv::Point p = projectToDisplay(v);
    return cv::Point2f( float(p.x) / (size().width()-1), float(p.y) / (size().height()-1));
}   // end projectToDisplayProportion
