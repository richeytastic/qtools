# =============================================================================
# The QTools CMake configuration file.
#
#           ** File generated automatically, DO NOT MODIFY! ***

# To use from an external project, in your project's CMakeLists.txt add:
#   FIND_PACKAGE( QTools REQUIRED)
#   INCLUDE_DIRECTORIES( QTools ${QTools_INCLUDE_DIRS})
#   LINK_DIRECTORIES( ${QTools_LIBRARY_DIR})
#   TARGET_LINK_LIBRARIES( MY_TARGET_NAME ${QTools_LIBRARIES})
#
# This module defines the following variables:
#   - QTools_FOUND         : True if QTools is found.
#   - QTools_ROOT_DIR      : The root directory where QTools is installed.
#   - QTools_INCLUDE_DIRS  : The QTools include directories.
#   - QTools_LIBRARY_DIR   : The QTools library directory.
#   - QTools_LIBRARIES     : The QTools imported libraries to link to.
#
# =============================================================================

get_filename_component( QTools_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component( QTools_ROOT_DIR  "${QTools_CMAKE_DIR}"           PATH)

set( QTools_INCLUDE_DIRS "${QTools_ROOT_DIR}/../include" CACHE PATH "The QTools include directories.")
set( QTools_LIBRARY_DIR  "${QTools_ROOT_DIR}"            CACHE PATH "The QTools library directory.")

include( "${CMAKE_CURRENT_LIST_DIR}/Macros.cmake")
get_library_suffix( _lsuff)
set( _hints QTools${_lsuff} libQTools${_lsuff})
find_library( QTools_LIBRARIES NAMES ${_hints} PATHS "${QTools_LIBRARY_DIR}/static" "${QTools_LIBRARY_DIR}")
set( QTools_LIBRARIES     ${QTools_LIBRARIES}         CACHE FILEPATH "The QTools imported libraries to link to.")

# handle QUIETLY and REQUIRED args and set QTools_FOUND to TRUE if all listed variables are TRUE
include( "${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake")
find_package_handle_standard_args( QTools QTools_FOUND QTools_LIBRARIES QTools_INCLUDE_DIRS)
