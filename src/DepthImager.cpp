#include "DepthImager.h"
using QTools::DepthImager;
#include "ui_DepthImager.h"
#include <cmath>
#include <cassert>
#include <iostream>
using std::cerr;
using std::endl;


DepthImager::DepthImager(QWidget *parent) :
    QGroupBox(parent), ui(new Ui::DepthImager)
{
    ui->setupUi(this);

    connect( this, SIGNAL( toggled(bool)), this, SLOT( updateImage()));
    connect( ui->showDepthRadioButton, SIGNAL( toggled(bool)), this, SLOT( updateImage()));
    connect( ui->showDepthD1RadioButton, SIGNAL( toggled(bool)), this, SLOT( updateImage()));
    connect( ui->showDepthD2RadioButton, SIGNAL( toggled(bool)), this, SLOT( updateImage()));
    connect( ui->scaleKernelCheckBox, SIGNAL( toggled(bool)), this, SLOT( updateImage()));
    connect( ui->kernelWidthSpinBox, SIGNAL( valueChanged(double)), this, SLOT( updateImage()));
    connect( ui->kernelHeightSpinBox, SIGNAL( valueChanged(double)), this, SLOT( updateImage()));
    connect( ui->matchKernelCheckBox, SIGNAL( toggled(bool)), this, SLOT( on_matchKernelCheckBox_toggled(bool)));
    modelSizeSet = false;
}   // end ctor


DepthImager::~DepthImager()
{
    delete ui;
}   // end dtor


void DepthImager::setView( const View::Ptr &v)
{
    _view = v;
    _dfinder = DepthFinder::Ptr(new DepthFinder( v->points, v->focalVec));
}   // end setView


void DepthImager::setModelSize( const cv::Size2f &ms)
{
    modelSizeSet = true;
    modelSize = ms;
    ui->matchKernelCheckBox->setEnabled(true);
}   // end setModelSize



bool DepthImager::checkViewState()
{
    if ( _view == NULL)
    {
        cerr << "Warning - cannot create D1 depth image with NULL view!" << endl;
        return false;
    }   // end if
    return true;
}   // end checkViewState



cv::Size2f DepthImager::getModelSize()
{
    if ( ui->matchKernelCheckBox->isChecked())
        return modelSize;
    return cv::Size2f( ui->kernelWidthSpinBox->value(), ui->kernelHeightSpinBox->value());
}   // end getModelSize



void DepthImager::showDepth()
{
    dimg = RFeatures::makeDisplayableRangeMap( _view->rngImg);
}   // end showDepth



void DepthImager::showDepthD1()
{
    const IntegralImage<double>::Ptr r1 = RangeGradientsBuilder( _view->rngImg, 9, false, false).getIntegralGradients();
    const cv::Mat_<double> rimg = createRangeChangeImage( r1);
    cv::Mat_<float> r2;
    rimg.convertTo(r2, CV_32F);
    dimg = RFeatures::makeDisplayableRangeMap( r2);
}   // end showDepthD1



void DepthImager::showDepthD2()
{
}   // end showDepthD2



cv::Mat_<float> DepthImager::createRangeChangeImage( const IntegralImage<double>::Ptr &srcDepth)
{
    const cv::Size imgSz = _view->img2d.size();
    const cv::Rect imgRect(0,0, imgSz.height, imgSz.width);    // For intersections
    const cv::Size2f modSize = getModelSize();

    cv::Mat_<float> outImg = cv::Mat_<float>::zeros( imgSz);  // For output
    cv::Rect mrect(0,0,1,1);

    // Snake down the rows
    for ( int i = 0; i < imgSz.height; ++i)
    {
        int j = 0;  // Even rows
        bool evenRow = true;
        if ( i % 2 == 1)
        {   // Odd rows
            j = imgSz.width - 1;
            evenRow = false;
        }   // end if

        mrect.y = i;

        int numCols = imgSz.width; // Column counter
        while ( numCols > 0)    // Across the columns
        {
            mrect.x = j;
            numCols--;

            if ( modSize.width > 0 && modSize.height > 0 && ui->scaleKernelCheckBox->isChecked())
                _dfinder->calcModelRect(modSize, j, i, mrect, 10);
            mrect &= imgRect;   // Only want the intersection

            /*
            // Check the corners of the rectangle to make an approximate check to see if the rectangle
            // overlaps any invalid (no range available) parts of the image (for example as caused by
            // discontinuities between the foreground and the background sky). If so, continue.
            if ( _view->rngImg.at<float>(mrect.y + mrect.height/2, mrect.x + mrect.width/2) == 0   // mrect centre
                || _view->rngImg.at<float>(mrect.y, mrect.x) == 0 // top left
                || _view->rngImg.at<float>(mrect.y, mrect.x + mrect.width) == 0   // top right
                || _view->rngImg.at<float>(mrect.y + mrect.height, mrect.x) == 0  // bottom left
                || _view->rngImg.at<float>(mrect.y + mrect.height, mrect.x + mrect.width) == 0)   // bottom right
                continue;
            */

            // Ignore where no valid range data is found (e.g. sky)
            int rngCnt = _dfinder->getValidRangeCount( mrect);
            if ( rngCnt > 0)
            { // Get the average over the area of valid range values given by mrect
                const float rngChng = float((*srcDepth)( mrect, 0)/rngCnt);
                outImg( mrect) += rngChng;
            }   // end if

            j = j + (evenRow ? 1 : -1);
        }   // end while
    }   // end for

    return outImg;
}   // end createRangeChangeImage



void DepthImager::updateImage()
{
    if ( isChecked() && checkViewState())
    {
        if ( ui->showDepthRadioButton->isChecked())
        {
            showDepth();
            ui->kernelHeightSpinBox->setEnabled(false);
            ui->kernelWidthSpinBox->setEnabled(false);
            ui->scaleKernelCheckBox->setEnabled(false);
            ui->matchKernelCheckBox->setEnabled(false);
        }   // end if
        else
        {
            bool matchKernelChecked = ui->matchKernelCheckBox->isChecked();
            ui->kernelHeightSpinBox->setEnabled(!matchKernelChecked);
            ui->kernelWidthSpinBox->setEnabled(!matchKernelChecked);
            ui->scaleKernelCheckBox->setEnabled(true);
            ui->matchKernelCheckBox->setEnabled(modelSizeSet);

            if ( ui->showDepthD1RadioButton->isChecked())
                showDepthD1();
            else if ( ui->showDepthD2RadioButton->isChecked())
                showDepthD2();
            else
            {
                cerr << "No other depth creation options available!" << endl;
                assert(true);
            }   // end else
        }   // end else
    }   // end if

    emit updated( isChecked());
}   // end updateImage



void DepthImager::on_matchKernelCheckBox_toggled( bool val)
{
    ui->kernelHeightSpinBox->setEnabled( !val);
    ui->kernelWidthSpinBox->setEnabled( !val);
    updateImage();
}   // end on_matchKernelCheckBox_toggled

