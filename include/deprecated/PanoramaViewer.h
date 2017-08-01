#pragma once
#ifndef QTOOLS_PANORAMA_VIEWER_H
#define QTOOLS_PANORAMA_VIEWER_H

#include <QWidget>
#include "ViewFrame.h"
#include "ViewNavigator.h"
using QTools::ViewNavigator;

#include "QTools_Export.h"
#include <DataTools.h>  // For extraction of features
using RFeatures::FeatureWriter;
#include <PointCloudOrienter.h>
using RFeatures::PointCloudOrienter;

#include <string>
using std::string;


namespace Ui {
class PanoramaViewer;
}

namespace QTools
{

class QTools_EXPORT PanoramaViewer : public QWidget
{ Q_OBJECT
public:
    static void displayKeyboardCommands( ostream& os);

    PanoramaViewer( const string &gtDir, QWidget *parent = 0);
    ~PanoramaViewer();

    // Get the current view's image (cloned)
    cv::Mat getViewImage() const;

    // Get the currently displayed view (may be NULL if no view currently displayed)
    View::Ptr getCurrentView() const;

    void setGroundTruthDir( const string& gtDir);
    string getGroundTruthDir() const { return _featureWriter->getGroundTruthDir();}

    // Get the current view face (front, left, right, back)
    ViewNavigator::ViewFace getCurrentViewFace() const;

    // Returns true iff point data is available.
    bool gotPoints() const;

    // Temporarily set an image for the view that is not the original view image.
    // New views or switching view will erase this image from the display.
    void setTempNonViewImage( const cv::Mat&);

    // Reset the view image to its original form
    void resetViewImage();

    void setModelSize( const cv::Size2f&);

public slots:
    void enableNavigation( bool enabled);
    void setPanorama( const Panorama::Ptr pano, const string& dataId);
    void setImage( const cv::Mat& img, const string& dataId);

signals:
    void showingUserRectangle( cv::Rect) const;
    void viewChanged( ViewNavigator::ViewFace);    // Emitted whenever the view changes

protected:
    virtual void keyPressEvent( QKeyEvent* event);

private slots:
    void showView( ViewNavigator::ViewFace);
    void changeViewImage();
    void reset();   // Reset all views

    void selectFeatureClass();
    void createNewFeatureClass();
    void extractImage( const cv::Rect) const;
    void showingUserRectangleSlot( const cv::Rect);
    void extractFeature();
    void eraseFeature( const cv::Rect) const;
    void enableFeatureExtract( bool);

private:
    Ui::PanoramaViewer *_ui;

    QTools::ViewFrame *_viewFrame;
    QTools::ViewNavigator *_panoNav;
    Panorama::Ptr _pano;   // Complete panorama (with all point data)
    FeatureWriter *_featureWriter;
    string _dataId;

    void populateFeatureClassComboBox();
    void updateFeatureWriter();
    View::Ptr getView( ViewNavigator::ViewFace face) const;
};  // end class

}   // end namespace

#endif
