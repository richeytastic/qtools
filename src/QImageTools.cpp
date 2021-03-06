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

#include <QImageTools.h>
#include <QFileDialog>
#include <cassert>
typedef unsigned char byte;

// Returns the image in RGB888 format
QImage QTools::copyOpenCV2QImage( const cv::Mat &img)
{
    assert( img.depth() == CV_8U);
    const int numChannels = img.channels();
    assert( numChannels == 1 || numChannels == 3);
    int c0 = 0, c1 = 0, c2 = 0;
    if ( numChannels == 3)
    {
        c1 = 1;
        c2 = 2;
    }   // end if

    QImage qimg( img.cols, img.rows, QImage::Format_RGB888);

    for ( int i = 0; i < img.rows; ++i)
    {
        const byte* inScanline = img.ptr(i);
        byte* outScanline = qimg.scanLine(i);

        for ( int j = 0; j < img.cols; ++j)
        {
            // BGR (OpenCV) --> RGB (Qt)
            outScanline[3*j+2] = inScanline[j*numChannels+c0];
            outScanline[3*j+1] = inScanline[j*numChannels+c1];
            outScanline[3*j+0] = inScanline[j*numChannels+c2];
        }   // end for
    }   // end for

    return qimg;
}   // end copyOpenCV2QImage


cv::Mat QTools::copyQImage2OpenCV( const QImage &qimg)
{
    const int qDepth = qimg.depth()/8; // Step size in bytes
    int boffset = 0;
    int goffset = 1;
    int roffset = 2;
    if ( qDepth == 1)
    {
        boffset = 0;
        goffset = 0;
    }   // end if

    cv::Size sz( qimg.width(), qimg.height());
    cv::Mat img( sz, CV_8UC3);

    for ( int i = 0; i < sz.height; ++i)
    {
        const byte* inScanline = qimg.scanLine(i);   // Source scanline
        byte* outScanline = img.ptr(i); // Destination scanline

        for ( int j = 0; j < sz.width; ++j)
        {
            // RGB (Qt) --> BGR (OpenCV)
            const int ch = 3*j;
            outScanline[ch+0] = *(inScanline + boffset);
            outScanline[ch+1] = *(inScanline + goffset);
            outScanline[ch+2] = *(inScanline + roffset);

            inScanline += qDepth;
        }   // end for
    }   // end for

    return img;
}   // end copyQImage2OpenCV


namespace {
QString getSaveImageFilename( const QString savefname, const QWidget *prnt)
{
    QString fname = QFileDialog::getSaveFileName( const_cast<QWidget*>(prnt), "Save Image", savefname, "Image Files (*.jpg *.jpeg *.png *.gif *.bmp)", NULL, QFileDialog::DontUseNativeDialog);
    if ( !fname.isEmpty())
    {
        // Ensure fname has a valid image extension, otherwise set
        // the default format (jpeg)
        if  ( !fname.endsWith( ".jpg", Qt::CaseInsensitive)
            && !fname.endsWith( ".jpeg", Qt::CaseInsensitive)
            && !fname.endsWith( ".png", Qt::CaseInsensitive)
            && !fname.endsWith( ".gif", Qt::CaseInsensitive)
            && !fname.endsWith( ".bmp", Qt::CaseInsensitive))
            fname += ".jpg";
    }   // end if
    return fname;
}   // end getSaveImageFilename
}   // end namespace


bool QTools::saveImage( const cv::Mat &img, const QString savefname, const QWidget *prnt)
{
    bool saved = false;
    const QString fname = getSaveImageFilename( savefname, prnt);
    if ( !fname.isEmpty())
        saved = cv::imwrite( fname.toStdString(), img);
    return saved;
}   // end saveImage


bool QTools::saveImage( const QImage& img, const QString savefname, const QWidget *prnt)
{
    bool saved = false;
    const QString fname = getSaveImageFilename( savefname, prnt);
    if ( !fname.isEmpty())
        saved = img.save( fname);
    return saved;
}   // end saveImage
