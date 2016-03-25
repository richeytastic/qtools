#pragma once
#ifndef QTOOLS_DEPTH_IMAGER_H
#define QTOOLS_DEPTH_IMAGER_H

#include <opencv2/opencv.hpp>
#include <View.h>
using RFeatures::View;
#include <DepthFinder.h>
using RFeatures::DepthFinder;
#include <RangeGradientsBuilder.h>
using RFeatures::RangeGradientsBuilder;
#include <IntegralImage.h>
using RFeatures::IntegralImage;
#include <QGroupBox>

namespace Ui {
    class DepthImager;  // UI
}   // end namespace


namespace QTools
{

class DepthImager : public QGroupBox
{ Q_OBJECT

public:
    explicit DepthImager( QWidget *parent = 0);
    ~DepthImager();

    // Get the most recently created depth image
    inline cv::Mat_<byte> getImage() const { return dimg;}

    void setView( const View::Ptr&);
    void setModelSize( const cv::Size2f&);

signals:
    // Always emitted at the end of updateImage. Value indicates whether
    // depth imaging is enabled (true) or not (false).
    void updated( bool);

private slots:
    void on_matchKernelCheckBox_toggled(bool);
    void updateImage();

private:
    Ui::DepthImager *ui;
    View::Ptr _view; // Current view
    DepthFinder::Ptr _dfinder;  // Depth finder for current view

    cv::Size2f modelSize;
    bool modelSizeSet;  // True if model size set

    cv::Mat_<byte> dimg;   // The current depth image

    // Check that view has been set
    bool checkViewState();
    cv::Size2f getModelSize();  // Get the model size (either user set or from modelSize)
    cv::Mat_<float> createRangeChangeImage( const IntegralImage<double>::Ptr &srcData);    // Generic image creation method

    // Select different depth images based on value, D1 and D2
    void showDepth();
    void showDepthD1();
    void showDepthD2();
};  // end class


}   // end namespace

#endif
