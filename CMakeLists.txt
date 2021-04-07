cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

project( QTools)

set( WITH_QT TRUE)
set( WITH_RIMG TRUE)
set( WITH_R3DVIS TRUE)
set( WITH_QUAZIP TRUE)
include( "cmake/FindLibs.cmake")

set( INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set( INCLUDE_F "${INCLUDE_DIR}/${PROJECT_NAME}")
set( SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set( FORMS_DIR "${PROJECT_SOURCE_DIR}/forms")
include_directories( ${INCLUDE_F})
include_directories( ${INCLUDE_DIR})
include_directories( ${PROJECT_BINARY_DIR}) # Req. for generated Qt form headers

add_subdirectory("tools/rmv")

set( SRC_FILES
    "${SRC_DIR}/AppUpdater.cpp"
    "${SRC_DIR}/ColourMappingWidget.cpp"
    "${SRC_DIR}/EventSignaller.cpp"
    "${SRC_DIR}/FileIO.cpp"
    "${SRC_DIR}/HelpAssistant.cpp"
    "${SRC_DIR}/HelpBrowser.cpp"
    #"${SRC_DIR}/ImagerWidget.cpp"
    "${SRC_DIR}/KeyPressHandler.cpp"
    "${SRC_DIR}/NetworkUpdater.cpp"
    "${SRC_DIR}/PatchList.cpp"
    "${SRC_DIR}/PluginInterface.cpp"
    "${SRC_DIR}/PluginsDialog.cpp"
    "${SRC_DIR}/PluginsLoader.cpp"
    "${SRC_DIR}/PluginUIPoints.cpp"
    "${SRC_DIR}/PolyDrawer.cpp"
    "${SRC_DIR}/QImageTools.cpp"
    "${SRC_DIR}/QUtils.cpp"
    "${SRC_DIR}/RangeMinMax.cpp"
    "${SRC_DIR}/RangeSlider.cpp"
    "${SRC_DIR}/ScalarColourRangeMapper.cpp"
    "${SRC_DIR}/TreeComboBox.cpp"
    "${SRC_DIR}/TreeItem.cpp"
    "${SRC_DIR}/TreeModel.cpp"
    "${SRC_DIR}/TwoHandleSlider.cpp"
    "${SRC_DIR}/ViewNavigator.cpp"
    "${SRC_DIR}/VtkActorViewer.cpp"
    "${SRC_DIR}/VtkViewerInteractorManager.cpp"
    "${SRC_DIR}/VtkViewerActorInteractor.cpp"
    "${SRC_DIR}/VtkViewerCameraInteractor.cpp"
    "${SRC_DIR}/VtkViewerSwitchInteractor.cpp"
    )

set( QOBJECTS
    "${INCLUDE_F}/AppUpdater.h"
    "${INCLUDE_F}/ColourMappingWidget.h"
    "${INCLUDE_F}/EventSignaller.h"
    "${INCLUDE_F}/HelpBrowser.h"
    #"${INCLUDE_F}/ImagerWidget.h"
    "${INCLUDE_F}/NetworkUpdater.h"
    "${INCLUDE_F}/VtkActorViewer.h"
    "${INCLUDE_F}/PluginInterface.h"
    "${INCLUDE_F}/PluginsDialog.h"
    "${INCLUDE_F}/PluginsLoader.h"
    "${INCLUDE_F}/PolyDrawer.h"
    "${INCLUDE_F}/RangeMinMax.h"
    "${INCLUDE_F}/RangeSlider.h"
    "${INCLUDE_F}/TreeComboBox.h"
    "${INCLUDE_F}/TreeModel.h"
    "${INCLUDE_F}/TwoHandleSlider.h"
    "${INCLUDE_F}/ViewNavigator.h"
    )

# Super set of header files
set( INCLUDE_FILES
    "${QOBJECTS}"
    "${INCLUDE_F}.h"
    "${INCLUDE_F}/FileIO.h"
    "${INCLUDE_F}/HelpAssistant.h"
    "${INCLUDE_F}/KeyPressHandler.h"
    "${INCLUDE_F}/PatchList.h"
    "${INCLUDE_F}/PluginUIPoints.h"
    "${INCLUDE_F}/QImageTools.h"
    "${INCLUDE_F}/QUtils.h"
    "${INCLUDE_F}/ScalarColourRangeMapper.h"
    "${INCLUDE_F}/TreeItem.h"
    "${INCLUDE_F}/VtkViewerInteractorManager.h"
    "${INCLUDE_F}/VtkViewerActorInteractor.h"
    "${INCLUDE_F}/VtkViewerCameraInteractor.h"
    "${INCLUDE_F}/VtkViewerSwitchInteractor.h"
    "${INCLUDE_F}/VtkMouseHandler.h"
    "${INCLUDE_F}/VtkViewerInteractor.h"
    )

set( FORMS
    "${FORMS_DIR}/ColourMappingWidget.ui"
    #"${FORMS_DIR}/ImagerWidget.ui"
    "${FORMS_DIR}/PluginsDialog.ui"
    "${FORMS_DIR}/ViewNavigator.ui"
    )

set( RCC_FILE "QTools_res.qrc")  # Qt resource file

qt5_wrap_ui( FORM_HEADERS ${FORMS})
qt5_wrap_cpp( QOBJECT_MOCS ${QOBJECTS})
qt5_add_resources( RESOURCE_FILE "${RCC_FILE}")

add_library( ${PROJECT_NAME} ${SRC_FILES} ${QOBJECT_MOCS} ${INCLUDE_FILES} ${FORM_HEADERS} ${FORMS} ${RESOURCE_FILE} ${RCC_FILE})
include( "cmake/LinkLibs.cmake")

if(UNIX)
    install( PROGRAMS "${PROJECT_SOURCE_DIR}/appimagetool-x86_64.AppImage" DESTINATION "bin")
endif()
