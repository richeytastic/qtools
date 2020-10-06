PROJECT(rmv)

add_executable(${PROJECT_NAME} main.cpp)
 
target_link_libraries( ${PROJECT_NAME} Qt5::Core)

install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin"
                                 LIBRARY DESTINATION "lib"
                                 ARCHIVE DESTINATION "lib/static")

if(UNIX)
    set_target_properties( ${PROJECT_NAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
    #[[
    file( GLOB _libgthread "/usr/lib/x86_64-linux-gnu/libgthread-2.0.so.0*")
    file( GLOB _libicuuc   "${QT_LIB_DIR}/libicuuc.so.*")
    file( GLOB _libicudata "${QT_LIB_DIR}/libicudata.so.*")
    file( GLOB _libicui18n "${QT_LIB_DIR}/libicui18n.so.*")
    file( GLOB _libQt5Core "${QT_LIB_DIR}/libQt5Core.so.*")
    install( FILES ${_libgthread} DESTINATION "lib")
    install( FILES ${_libicuuc}   DESTINATION "lib")
    install( FILES ${_libicudata} DESTINATION "lib")
    install( FILES ${_libicui18n} DESTINATION "lib")
    install( FILES ${_libQt5Core} DESTINATION "lib")

    # Use the full RPATH for the build version.
    set_target_properties( ${PROJECT_NAME} PROPERTIES SKIP_BUILD_RPATH FALSE)

    # The RPATH for the installed version should reference copies of the libs
    # it needs relative to its installed location.
    set_target_properties( ${PROJECT_NAME} PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib")

    # Don't use the installation RPATH for the build version though.
    set_target_properties( ${PROJECT_NAME} PROPERTIES BUILD_WITH_INSTALL_RPATH FALSE)

    # The below does the same thing but via external command (needs patchelf to be available).
    #install( CODE "execute_process( COMMAND patchelf --set-rpath $ORIGIN/../lib ${PROJECT_NAME}
    #                                WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)")
    #]]
elseif(WIN32)
    set( QT_BIN "${Qt5_DIR}/../../../bin")
    install( FILES "${QT_BIN}/Qt5Core.dll"  DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endif()

