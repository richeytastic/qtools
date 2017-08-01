#include "PointCloudViewer.h"
using QTools::PointCloudViewer;
#include "ui_PointCloudViewer.h"

#include <QMenu>
#include <QMouseEvent>

#include <cassert>
#include <sstream>
using std::ostringstream;



PointCloudViewer::PointCloudViewer( QWidget *parent)
    : QWidget( parent), ui( new Ui::PointCloudViewer),
    toggleBg( NULL), toggleSt( NULL), togglePr( NULL),
    connections( NULL), vtkViewer( Viewer::create())
{
    pointCloudActor = VtkDisplayActor::create( vtkViewer);

    QWidget::setWindowFlags(Qt::Window);
    ui->setupUi( this);
    ui->qvtkWidget->SetRenderWindow( vtkViewer->getRenderWindow());

    connect( ui->backgroundCheckBox, SIGNAL( clicked(bool)), this, SLOT( setBackgroundWhite(bool)));
    connect( ui->parallelCheckBox, SIGNAL( clicked(bool)), this, SLOT( setOrthogonal(bool)));
    connect( ui->stereoCheckBox, SIGNAL( clicked(bool)), this, SLOT( setStereoRendering(bool)));
    connect( ui->resetCameraButton, SIGNAL( clicked()), this, SLOT( resetCamera()));
    connect( ui->saveImageButton, SIGNAL( clicked()), this, SLOT( saveImage()));

    createContextMenu();
    connectMouseEvents();
}	// end ctor



PointCloudViewer::~PointCloudViewer()
{
    clear();
    if ( ui != NULL)
        delete ui;
    if ( connections != NULL)
        connections->Delete();
}	// end dtor



void PointCloudViewer::setViewArea( int w, int h)
{
    ui->qvtkWidget->resize(w,h);
    ui->qvtkWidget->setMinimumSize(w,h);
    ui->qvtkWidget->setMaximumSize(w,h);
}   // end setViewArea



cv::Mat_<cv::Vec3b> PointCloudViewer::getImage() const
{
    //const QSize size = ui->qvtkWidget->size();
    const int *size = ui->qvtkWidget->GetRenderWindow()->GetSize();
    // Return a newly allocated array of pixels where each pixel is stored in
    // RGB order and memory organisation is from BOTTOM left to TOP right.
    const int cols = size[0];
    const int rows = size[1];
    const unsigned char *pxlData = ui->qvtkWidget->GetRenderWindow()->GetPixelData(0,0,cols-1,rows-1,0);
    cv::Mat_<cv::Vec3b> img(rows, cols);    // Continuous
    assert( img.isContinuous());
    unsigned char *imgPtr = (unsigned char*)img.ptr<cv::Vec3b>();
    const int totSz = rows*cols;
    for ( int i = 0; i < totSz; ++i)   // From BOTTOM left of image to top right
    {
        const int pxIdx = 3*i;
        // Reverse RGB to be BGR for OpenCV
        imgPtr[pxIdx] = pxlData[pxIdx+2];
        imgPtr[pxIdx+1] = pxlData[pxIdx+1];
        imgPtr[pxIdx+2] = pxlData[pxIdx];
    }   // end for
    delete pxlData; // Not needed any longer

    // Flip vertically because bytes read in from bottom to top
    cv::flip( img, img, 0);
    return img;
}   // end getImage



void PointCloudViewer::setBackgroundWhite( bool on)
{
    ui->backgroundCheckBox->setChecked(on);
    toggleBg->setChecked(on);
    vtkViewer->changeBackground( on ? 255 : 0);
    updateRender();
}	// end setBackgroundWhite



void PointCloudViewer::setStereoRendering( bool on)
{
    ui->stereoCheckBox->setChecked(on);
    toggleSt->setChecked(on);
    vtkViewer->setStereoRendering( on);
    updateRender();
}	// end setStereoRendering



void PointCloudViewer::setOrthogonal( bool on)
{
    ui->parallelCheckBox->setChecked(on);
    togglePr->setChecked(on);
    vtkViewer->setPerspective( !on);
    if ( on)
        vtkViewer->setParallelScale( 10.0);    // For parallel projection
    else
        vtkViewer->setFieldOfView( 90.0);  // For perspective projection
    updateRender();
}	// end setPerspective



void PointCloudViewer::resetCamera()
{
    vtkViewer->setCameraPosition( camPos);
    vtkViewer->setCameraFocus( camFocus);
    vtkViewer->setCameraViewUp( camUp);
    updateRender();
}   // end resetCamera



void PointCloudViewer::saveCameraParameters( const cv::Vec3d &pos, const cv::Vec3d &focus, const cv::Vec3d &up)
{
    camPos = pos;
    camFocus = focus;
    camUp = up;
}   // end saveCameraParameters



void PointCloudViewer::resetPointCloud( const cv::Vec3d &pos, const cv::Vec3d &foc, const cv::Vec3d &up)
{
    pointCloudActor->setPosition( pos);
    saveCameraParameters( pos, foc, up);
    pointCloudActor->show();
    resetCamera();
}   // end resetPointCloud



void PointCloudViewer::addPoints( const cv::Mat_<cv::Vec3f> pc, const cv::Mat_<cv::Vec3b> ci)
{
    if ( pointCloudMapper == NULL)
    {
        pointCloudMapper = PointCloudMapper::create();
        pointCloudActor->setMapper( pointCloudMapper->getMapper());
    }   // end if
    pointCloudMapper->add( pc, ci);
    updateRender();
}   // end addPoints



void PointCloudViewer::setProgressBar( uint pcnt, uint count)
{
    ostringstream oss;
    oss << count << " points";
    ui->progressBar->setFormat(oss.str().c_str());
    ui->progressBar->setValue(pcnt);
}   // end setProgressBar



#include <LinesConverter.h>
void PointCloudViewer::setLines( const Lines &lns, const cv::Mat &rngData, int minSegLen, double minFitCoeff)
{
    if ( lns.empty())
    {
        linesActor.reset();
        linesMapper.reset();
    }   // end if
    else
    {
        RFeatures::LinesConverter conv( lns, rngData, minSegLen, 256);  // Try 1?
        // If we're not depth filtering, we simply convert the 2D lines to 3D.
        Lines3d lns3d;
        if ( minFitCoeff < 0)
            lns3d = conv.get3DLines();
        else
            lns3d = conv.fitAndFilter( minFitCoeff);

        cv::Vec3d pos(0,0,0);
        linesActor = VtkDisplayActor::create( vtkViewer);
        linesActor->setPosition( pos);
        linesMapper = LinesMapper::create( lns3d);
        linesActor->setMapper( linesMapper->getMapper());
        linesActor->show();
    }   // end else

    updateRender();
}   // end setLines


void PointCloudViewer::setCircles( Circles &circles)
{
}   // end setCircles


void PointCloudViewer::setOpaque( VtkDisplayActor::Ptr act)
{
    if ( linesActor == NULL && act != NULL)
        act->setOpacity(1.0);
}   // end setOpaque



void PointCloudViewer::setTransparent( VtkDisplayActor::Ptr act)
{
    const float ACTOR_TRANSPARENCY = 0.33;
    act->setOpacity( ACTOR_TRANSPARENCY);
}   // end setTransparent



void PointCloudViewer::clear()
{
    pointCloudMapper = PointCloudMapper::create();
    pointCloudActor->setMapper( pointCloudMapper->getMapper());
    pointCloudActor->hide();
    pointCloudMapper.reset();
    ui->progressBar->setValue(0);
    setBackgroundWhite(false);
    setOrthogonal(false);
    setStereoRendering(false);
}	// end clear



void PointCloudViewer::updateRender()
{
    vtkViewer->updateRender();
}   // end updateRender



void PointCloudViewer::updateCoords( vtkObject *obj)
{
    vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::SafeDownCast( obj);
    int event_pos[2];
    iren->GetEventPosition( event_pos);
    int xpos = -1;
    int ypos = -1;
    vtkRenderer *ren = ui->qvtkWidget->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    if (( event_pos[0] < ren->GetSize()[0]) && ( event_pos[0] >= 0))
        xpos = event_pos[0];
    if (( event_pos[1] < ren->GetSize()[1]) && ( event_pos[1] >= 0))
        ypos = event_pos[1];
    //emit updateCoords( xpos, ypos);
}	// end updateCoords



void PointCloudViewer::displayContextMenu( vtkObject *obj, unsigned long, void *clientData, void*, vtkCommand *command)
{
    vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::SafeDownCast( obj);
    command->AbortFlagOn();	// Prevent the interactor from getting the event
    QMenu *popupMenu = static_cast<QMenu*>( clientData);
    int *sz = iren->GetSize();
    int *position = iren->GetEventPosition();
    QPoint pt = QPoint(position[0], sz[1]-position[1]);	// Flip Y axis
    QPoint globalPt = popupMenu->parentWidget()->mapToGlobal(pt);
    popupMenu->popup( globalPt);	// Show popup menu at global point
}	// end displayContextMenu



void PointCloudViewer::saveImage() const
{
    const cv::Mat_<cv::Vec3b> img = getImage();
    QImageTools::saveImage(img);
}   // end saveImage



void PointCloudViewer::createContextMenu()
{
    toggleBg = new QAction( tr("Background (White/Black)"), this);
    toggleBg->setToolTip( tr("Change background between black and white"));
    toggleBg->setCheckable(true);
    toggleBg->setChecked(false);
    connect( toggleBg, SIGNAL(triggered(bool)), this, SLOT( setBackgroundWhite(bool)));

    toggleSt = new QAction( tr("Stereo Rendering (On/Off)"), this);
    toggleSt->setToolTip( tr("Display using stereo rendering (requires 3D glasses)"));
    toggleSt->setCheckable(true);
    toggleSt->setChecked(false);
    connect( toggleSt, SIGNAL(triggered(bool)), this, SLOT( setStereoRendering(bool)));

    togglePr = new QAction( tr("Orthogonal Projection (On/Off)"), this);
    togglePr->setToolTip( tr("Toggle between parallel and perspective projection modes"));
    togglePr->setCheckable(true);
    togglePr->setChecked(false);
    connect( togglePr, SIGNAL(triggered(bool)), this, SLOT( setOrthogonal(bool)));

    QMenu *contextMenu = new QMenu(this);
    contextMenu->addAction( toggleBg);
    contextMenu->addAction( toggleSt);
    contextMenu->addAction( togglePr);

    connections = vtkEventQtSlotConnect::New();
    vtkRenderWindowInteractor *iren = ui->qvtkWidget->GetRenderWindow()->GetInteractor();

    // Connect up the VTK right mouse button press event to displayContextMenu
    connections->Connect( iren, vtkCommand::RightButtonPressEvent, this,
                        SLOT( displayContextMenu( vtkObject*, unsigned long, void*, void*, vtkCommand*)),
                        contextMenu, 1.0);
}	// end createContextMenu



void PointCloudViewer::connectMouseEvents()
{
    vtkRenderWindowInteractor *iren = ui->qvtkWidget->GetRenderWindow()->GetInteractor();

    // Update coordinates with x and y pixel coords when mouse
    // moves through render window or leaves its confines.
    connections->Connect( iren, vtkCommand::MouseMoveEvent, this, SLOT( updateCoords( vtkObject*)));
    connections->Connect( iren, vtkCommand::LeaveEvent, this, SLOT( updateCoords( vtkObject*)));
}	// end createMenus
