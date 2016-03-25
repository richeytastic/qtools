cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project( QTools)

include( "../LibCMakeCommon.cmake")
set( BUILD_SHARED_LIBS TRUE)

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
if(WIN32)
    set( BOOST_ROOT "D:/boost_1_56_0")
    set( BOOST_LIBRARYDIR "${BOOST_ROOT}/lib64-msvc-11.0")
endif(WIN32)
find_package( Boost 1.5 REQUIRED COMPONENTS filesystem system thread random)
include_directories( ${Boost_INCLUDE_DIRS})
link_directories( ${Boost_LIBRARY_DIRS})

# OpenCV
if(WIN32)
    set( OpenCV_DIR "${LOCALLIBS}/opencv")
endif(WIN32)
find_package( OpenCV 2.4 REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS})
link_libraries( ${OpenCV_LIBS})

# VTK
set( VTK_DIR "${LOCALLIBS}/VTK64/lib/cmake/vtk-6.1")
find_package( VTK REQUIRED)
include( "${VTK_USE_FILE}")
include_directories( ${VTK_INCLUDE_DIRS})
link_libraries( ${VTK_LIBRARIES})
if(UNIX)
    link_libraries( QVTK)
endif()

# Qt
if(UNIX)
    find_package( Qt4 REQUIRED QtCore QtGui)
    include( ${QT_USE_FILE})
    qt4_wrap_ui( FORM_HEADERS ${FORMS})
    qt4_wrap_cpp( QOBJECT_MOCS ${QOBJECTS} OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
elseif(WIN32)
    # Build 64-bit using the MSVC11 64-bit compiler
    set( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "C:/Program Files (x86)/Windows Kits/8.0/Lib/win8/um/x64")
    set( Qt5_LIBRARY_DIR "D:/Qt/5.2.1/msvc2012_64_opengl/lib")
    find_package( Qt5 REQUIRED Widgets OpenGL Sql WebKitWidgets)
    include_directories( "${Qt5Widgets_INCLUDE_DIRS}")

    qt5_wrap_ui( FORM_HEADERS ${FORMS})
    qt5_wrap_cpp( QOBJECT_MOCS ${QOBJECTS})

    add_definitions( ${Qt5Widgets_DEFINITIONS})
    add_definitions( ${Qt5OpenGL_DEFINITIONS})
    add_definitions( ${Qt5Sql_DEFINITIONS})
    add_definitions( ${Qt5WebKitWidgets_DEFINITIONS})
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} ${Qt5Sql_EXECUTABLE_COMPILE_FLAGS}")
    set( QT_LIBRARIES Qt5::Widgets Qt5::OpenGL Qt5::Sql Qt5::WebKitWidgets)
endif()
link_directories( ${QT_LIBRARY_DIR})
link_libraries( ${QT_LIBRARIES})


set( RLIB_DIR "${LOCALLIBS}/rlib")
find_package( RLIB REQUIRED)
include_directories( ${RLIB_INCLUDE_DIR})
link_libraries( ${RLIB_LIBRARY})

set( RFeatures_DIR "${LOCALLIBS}/rFeatures")
find_package( RFeatures REQUIRED)
include_directories( ${RFeatures_INCLUDE_DIR})
link_libraries( ${RFeatures_LIBRARY})

set( RVTK_DIR "${LOCALLIBS}/rVTK")
find_package( RVTK REQUIRED)
include_directories( ${RVTK_INCLUDE_DIR})
link_libraries( ${RVTK_LIBRARY})

set( LibLAS_DIR "${LOCALLIBS}/libLAS-1.7.0")
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

set( CMAKE_INSTALL_PREFIX "${LOCALLIBS}/${PROJECT_NAME}")
install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin"
                                 LIBRARY DESTINATION "lib"
                                 ARCHIVE DESTINATION "lib/static")

install( FILES ${QOBJECTS} DESTINATION "include")
install( FILES ${HEADERS} DESTINATION "include")
install( FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}_Export.h" DESTINATION "include")
# Copy across this library's Find cmake module to ${LOCALLIBS}/cmake
install( FILES "${PROJECT_SOURCE_DIR}/../CMakeModules/FindQTools.cmake" DESTINATION "${LOCALLIBS}/cmake")

