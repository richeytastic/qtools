cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project( testSlider)

set( CMAKE_COLOR_MAKEFILE TRUE)
set( CMAKE_VERBOSE_MAKEFILE FALSE)

#set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

set( INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set( SRC_DIR "${PROJECT_SOURCE_DIR}/src")

include_directories( "${INCLUDE_DIR}")
include_directories( "${PROJECT_BINARY_DIR}/include")
include_directories( "${CMAKE_CURRENT_BINARY_DIR}")

set( SOURCES "${SRC_DIR}/main.cpp"
             "${SRC_DIR}/MainWindow.cpp"
             "${SRC_DIR}/TwoHandleSlider.cpp"
             )

set( QOBJECTS "${INCLUDE_DIR}/MainWindow.h"
              "${INCLUDE_DIR}/TwoHandleSlider.h"
              )

set( HEADERS ${QOBJECTS}
             )

set( RESOURCE_FILE "res.qrc")

# Qt
find_package( Qt5 REQUIRED Widgets Sql)
include_directories( "${Qt5Widgets_INCLUDE_DIRS}")
qt5_wrap_cpp( QOBJECT_MOCS ${QOBJECTS})
qt5_add_resources( RESOURCE_FILE ${RESOURCE_FILE})
add_definitions( ${Qt5Widgets_DEFINITIONS})
add_definitions( ${Qt5Sql_DEFINITIONS})
set( QT_LIBRARIES Qt5::Widgets Qt5::Sql)


add_executable( ${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${QOBJECT_MOCS} ${RESOURCE_FILE})
target_link_libraries( ${PROJECT_NAME} ${QT_LIBRARIES})

