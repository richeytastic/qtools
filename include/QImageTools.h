#ifndef QTOOLS_QIMAGE_TOOLS_H
#define QTOOLS_QIMAGE_TOOLS_H

#include <opencv2/opencv.hpp>
#include <QImage>
#include <QString>
#include "QTools_Export.h"

namespace QTools
{

class QImageTools
{
public:
    // Copy a CV_8UC1 or CV_8UC3 cv::Mat to a RGB888 QImage
    static QTools_EXPORT QImage copyOpenCV2QImage( const cv::Mat &img);

    // Copy a RGB888 or RGB32 QImage to a BGR 3 channel 24 bit cv::Mat image
    static QTools_EXPORT cv::Mat copyQImage2OpenCV( const QImage &img);

    // Save the given image to a filename provided using file dialog.
    // Returns false only if tried and failed to save the image.
    static QTools_EXPORT bool saveImage( const cv::Mat &img, const QString savefname=QString());

    static QTools_EXPORT bool saveImage( const QImage& qimg, const QString savefname=QString());
};  // end class

}   // end namespace

#endif
