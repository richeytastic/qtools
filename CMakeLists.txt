cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project( QTools)

set( WITH_QT TRUE)
set( WITH_RVTK TRUE)
include( "cmake/LibCommon.cmake")

set( INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set( SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set( FORMS_DIR "${PROJECT_SOURCE_DIR}/forms")
include_directories( ${INCLUDE_DIR})

set( SRC_FILES
    "${SRC_DIR}/DatabaseClient.cpp"
    "${SRC_DIR}/DatabaseOptionsDialog.cpp"
    "${SRC_DIR}/ImagerWidget.cpp"
    "${SRC_DIR}/VtkActorViewer.cpp"
    #"${SRC_DIR}/PanoramaViewer.cpp"
    "${SRC_DIR}/PluginInterface.cpp"
    "${SRC_DIR}/PluginsLoader.cpp"
    #"${SRC_DIR}/PointCloudViewer.cpp"
    "${SRC_DIR}/PolyDrawer.cpp"
    "${SRC_DIR}/QImageTools.cpp"
    "${SRC_DIR}/QProgressBarUpdater.cpp"
    "${SRC_DIR}/QUtils.cpp"
    "${SRC_DIR}/TwoHandleSlider.cpp"
    #"${SRC_DIR}/ViewFrame.cpp"
    "${SRC_DIR}/ViewNavigator.cpp"
    )

set( QOBJECTS
    "${INCLUDE_DIR}/DatabaseOptionsDialog.h"
    "${INCLUDE_DIR}/ImagerWidget.h"
    "${INCLUDE_DIR}/VtkActorViewer.h"
    #"${INCLUDE_DIR}/PanoramaViewer.h"
    "${INCLUDE_DIR}/PluginInterface.h"
    "${INCLUDE_DIR}/PluginsLoader.h"
    #"${INCLUDE_DIR}/PointCloudViewer.h"
    "${INCLUDE_DIR}/PolyDrawer.h"
    "${INCLUDE_DIR}/QProgressBarUpdater.h"
    "${INCLUDE_DIR}/TwoHandleSlider.h"
    #"${INCLUDE_DIR}/ViewFrame.h"
    "${INCLUDE_DIR}/ViewNavigator.h"
    )

# Super set of header files
set( INCLUDE_FILES
    "${QOBJECTS}"
    "${INCLUDE_DIR}/DatabaseClient.h"
    "${INCLUDE_DIR}/QImageTools.h"
    "${INCLUDE_DIR}/QUtils.h"
    )

set( FORMS
    "${FORMS_DIR}/DatabaseOptionsDialog.ui"
    "${FORMS_DIR}/ImagerWidget.ui"
    #"${FORMS_DIR}/PanoramaViewer.ui"
    #"${FORMS_DIR}/PointCloudViewer.ui"
    #"${FORMS_DIR}/ViewFrame.ui"
    "${FORMS_DIR}/ViewNavigator.ui"
    )

set( RESOURCE_FILE "vres.qrc")  # Qt resource file

qt5_wrap_ui( FORM_HEADERS ${FORMS})
qt5_wrap_cpp( QOBJECT_MOCS ${QOBJECTS})
qt5_add_resources( RESOURCE_FILE "${RESOURCE_FILE}" OPTIONS "-no-compress")

add_library( ${PROJECT_NAME} ${SRC_FILES} ${QOBJECT_MOCS} ${INCLUDE_FILES} ${FORMS} ${RESOURCE_FILE})
include( "cmake/LibInstall.cmake")
