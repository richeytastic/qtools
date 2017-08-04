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
