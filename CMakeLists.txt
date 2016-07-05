cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project( QTools)

include( "../LibCMakeCommon.cmake")
set( BUILD_SHARED_LIBS TRUE)
set( BUILD_USING_SHARED_LIBS TRUE)

set( SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set( INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set( FORMS_DIR "${PROJECT_SOURCE_DIR}/forms")
include_directories( "${INCLUDE_DIR}")
# Include the build directory because generated header files sit here.
include_directories( "${PROJECT_BINARY_DIR}")
include_directories( "${CMAKE_CURRENT_BINARY_DIR}")

include_directories( "${PROJECT_BINARY_DIR}/include")

set( SRC_FILES
    "${SRC_DIR}/DatabaseClient.cpp"
    "${SRC_DIR}/DatabaseOptionsDialog.cpp"
    "${SRC_DIR}/ImagerWidget.cpp"
    "${SRC_DIR}/VtkActorViewer.cpp"
    "${SRC_DIR}/PanoramaViewer.cpp"
    "${SRC_DIR}/PluginInterface.cpp"
    "${SRC_DIR}/PluginsPool.cpp"
    "${SRC_DIR}/PointCloudViewer.cpp"
    "${SRC_DIR}/PolyDrawer.cpp"
    "${SRC_DIR}/QImageTools.cpp"
    "${SRC_DIR}/QProgressBarUpdater.cpp"
    "${SRC_DIR}/QUtils.cpp"
    "${SRC_DIR}/ViewFrame.cpp"
    "${SRC_DIR}/ViewNavigator.cpp"
    )

# Subset of header files that are also Q_OBJECTs
set( QOBJECTS
    "${INCLUDE_DIR}/DatabaseOptionsDialog.h"
    "${INCLUDE_DIR}/ImagerWidget.h"
    "${INCLUDE_DIR}/VtkActorViewer.h"
    "${INCLUDE_DIR}/PanoramaViewer.h"
    "${INCLUDE_DIR}/PluginInterface.h"
    "${INCLUDE_DIR}/PluginsPool.h"
    "${INCLUDE_DIR}/PointCloudViewer.h"
    "${INCLUDE_DIR}/PolyDrawer.h"
    "${INCLUDE_DIR}/QProgressBarUpdater.h"
    "${INCLUDE_DIR}/ViewFrame.h"
    "${INCLUDE_DIR}/ViewNavigator.h"
    )

# Super set of header files
set( HEADERS
    "${QOBJECTS}"
    "${INCLUDE_DIR}/DatabaseClient.h"
    "${INCLUDE_DIR}/QImageTools.h"
    "${INCLUDE_DIR}/QUtils.h"
    )

set( FORMS
    "${FORMS_DIR}/DatabaseOptionsDialog.ui"
    "${FORMS_DIR}/ImagerWidget.ui"
    "${FORMS_DIR}/PanoramaViewer.ui"
    "${FORMS_DIR}/PointCloudViewer.ui"
    "${FORMS_DIR}/ViewFrame.ui"
    "${FORMS_DIR}/ViewNavigator.ui"
    )


# Boost
find_package( Boost 1.5 REQUIRED COMPONENTS system filesystem thread random regex)
include_directories( ${Boost_INCLUDE_DIRS})
#add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})


# OpenCV
find_package( OpenCV 2.4 REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS})
link_libraries( ${OpenCV_LIBS})

# VTK
find_package( VTK REQUIRED)
include( "${VTK_USE_FILE}")
include_directories( ${VTK_INCLUDE_DIRS})
link_libraries( ${VTK_LIBRARIES})

# Qt
find_package( Qt5 REQUIRED Widgets OpenGL Sql)
include_directories( "${Qt5Widgets_INCLUDE_DIRS}")
qt5_wrap_ui( FORM_HEADERS ${FORMS})
qt5_wrap_cpp( QOBJECT_MOCS ${QOBJECTS})
add_definitions( ${Qt5Widgets_DEFINITIONS})
add_definitions( ${Qt5OpenGL_DEFINITIONS})
add_definitions( ${Qt5Sql_DEFINITIONS})
set( QT_LIBRARIES Qt5::Widgets Qt5::OpenGL Qt5::Sql)
link_directories( ${QT_LIBRARY_DIR})
link_libraries( ${QT_LIBRARIES})


find_package( RLIB REQUIRED)
include_directories( ${RLIB_INCLUDE_DIR})
link_libraries( ${RLIB_LIBRARY})

find_package( RFeatures REQUIRED)
include_directories( ${RFeatures_INCLUDE_DIR})
link_libraries( ${RFeatures_LIBRARY})

find_package( RVTK REQUIRED)
include_directories( ${RVTK_INCLUDE_DIR})
link_libraries( ${RVTK_LIBRARY})

find_package( LibLAS REQUIRED)
link_libraries( ${LibLAS_LIBRARY})


# Require generate_export_header for .lib import libraries associated with Windows DLLs (if BUILD_SHARED_LIBS is true)
include(GenerateExportHeader)
add_library( ${PROJECT_NAME} ${SRC_FILES} ${QOBJECT_MOCS} ${HEADERS} ${FORMS})
generate_export_header( ${PROJECT_NAME}
    BASE_NAME ${PROJECT_NAME}
    EXPORT_MACRO_NAME ${PROJECT_NAME}_EXPORT
    EXPORT_FILE_NAME ${PROJECT_NAME}_Export.h
    STATIC_DEFINE ${PROJECT_NAME}_BUILT_AS_STATIC)

target_link_libraries( ${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries( ${PROJECT_NAME} ${QT_LIBRARIES})

set( CMAKE_INSTALL_PREFIX "${LOCALLIBS}/${PROJECT_NAME}")
install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin"
                                 LIBRARY DESTINATION "lib"
                                 ARCHIVE DESTINATION "lib/static")

install( FILES ${QOBJECTS} DESTINATION "include")
install( FILES ${HEADERS} DESTINATION "include")
install( FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}_Export.h" DESTINATION "include")
# Copy across this library's Find cmake module to ${LOCALLIBS}/cmake
install( FILES "${PROJECT_SOURCE_DIR}/../CMakeModules/FindQTools.cmake" DESTINATION "${LOCALLIBS}/cmake")



