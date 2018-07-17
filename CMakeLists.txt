cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project( QTools)

set( WITH_QT TRUE)
set( WITH_RVTK TRUE)
include( "cmake/FindLibs.cmake")

set( INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set( SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set( FORMS_DIR "${PROJECT_SOURCE_DIR}/forms")
include_directories( ${INCLUDE_DIR})
include_directories( ${PROJECT_BINARY_DIR}) # Req. for generated Qt form headers

set( SRC_FILES
    "${SRC_DIR}/ColourMappingWidget.cpp"
    "${SRC_DIR}/DatabaseClient.cpp"
    "${SRC_DIR}/DatabaseOptionsDialog.cpp"
    "${SRC_DIR}/ImagerWidget.cpp"
    "${SRC_DIR}/KeyPressHandler.cpp"
    "${SRC_DIR}/PluginInterface.cpp"
    "${SRC_DIR}/PluginsDialog.cpp"
    "${SRC_DIR}/PluginsLoader.cpp"
    "${SRC_DIR}/PolyDrawer.cpp"
    "${SRC_DIR}/QImageTools.cpp"
    "${SRC_DIR}/QProgressUpdater.cpp"
    "${SRC_DIR}/QUtils.cpp"
    "${SRC_DIR}/TwoHandleSlider.cpp"
    "${SRC_DIR}/ViewNavigator.cpp"
    "${SRC_DIR}/VtkActorViewer.cpp"
    "${SRC_DIR}/VtkViewerInteractorManager.cpp"
    "${SRC_DIR}/VtkViewerActorInteractor.cpp"
    "${SRC_DIR}/VtkViewerCameraInteractor.cpp"
    "${SRC_DIR}/VtkViewerSwitchInteractor.cpp"
    )

set( QOBJECTS
    "${INCLUDE_DIR}/ColourMappingWidget.h"
    "${INCLUDE_DIR}/DatabaseOptionsDialog.h"
    "${INCLUDE_DIR}/ImagerWidget.h"
    "${INCLUDE_DIR}/VtkActorViewer.h"
    "${INCLUDE_DIR}/PluginInterface.h"
    "${INCLUDE_DIR}/PluginsDialog.h"
    "${INCLUDE_DIR}/PluginsLoader.h"
    "${INCLUDE_DIR}/PolyDrawer.h"
    "${INCLUDE_DIR}/QProgressUpdater.h"
    "${INCLUDE_DIR}/TwoHandleSlider.h"
    "${INCLUDE_DIR}/ViewNavigator.h"
    "${INCLUDE_DIR}/VtkViewerInteractor.h"
    )

# Super set of header files
set( INCLUDE_FILES
    "${QOBJECTS}"
    "${INCLUDE_DIR}/DatabaseClient.h"
    "${INCLUDE_DIR}/KeyPressHandler.h"
    "${INCLUDE_DIR}/QImageTools.h"
    "${INCLUDE_DIR}/QUtils.h"
    "${INCLUDE_DIR}/VtkViewerInteractorManager.h"
    "${INCLUDE_DIR}/VtkViewerActorInteractor.h"
    "${INCLUDE_DIR}/VtkViewerCameraInteractor.h"
    "${INCLUDE_DIR}/VtkViewerSwitchInteractor.h"
    )

set( FORMS
    "${FORMS_DIR}/ColourMappingWidget.ui"
    "${FORMS_DIR}/DatabaseOptionsDialog.ui"
    "${FORMS_DIR}/ImagerWidget.ui"
    "${FORMS_DIR}/PluginsDialog.ui"
    "${FORMS_DIR}/ViewNavigator.ui"
    )

set( RESOURCE_FILE "QTools_res.qrc")  # Qt resource file

qt5_wrap_ui( FORM_HEADERS ${FORMS})
qt5_wrap_cpp( QOBJECT_MOCS ${QOBJECTS})
qt5_add_resources( RESOURCE_FILE "${RESOURCE_FILE}")

add_library( ${PROJECT_NAME} ${SRC_FILES} ${QOBJECT_MOCS} ${INCLUDE_FILES} ${FORMS} ${RESOURCE_FILE})
include( "cmake/LinkLibs.cmake")
