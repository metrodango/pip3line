#
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information
#

cmake_minimum_required(VERSION 3.18)
message ( STATUS "Configuring the Python 3 plugin --" )
project( python3plugin )

set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DPYTHONPLUGIN_LIBRARY -DQT_NO_KEYWORDS -DBUILD_PYTHON_3)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Xlinker -export-dynamic")

find_package(Qt5 COMPONENTS Core Widgets Svg REQUIRED)

# no need for harcoded libs here, if none are supplied or found, failing the build

if (NOT PYTHON3_INCLUDE_DIRS OR NOT PYTHON3_LIBRARIES)
    find_package(Python COMPONENTS Development)
    if( NOT Python_FOUND)
        message ( FATAL_ERROR "Python 3 includes/libs haven't been found or supplied" )
    endif()
else()
    message ( STATUS "Python 3 include & libs paths have been supplied by user" )
    set (Python_INCLUDE_DIRS  ${PYTHON3_INCLUDE_DIRS})
    set (Python_LIBRARIES ${PYTHON3_LIBRARIES})
endif()

if (Python_INCLUDE_DIRS AND Python_LIBRARIES)
    message ( STATUS "Python 3 include: ${Python_INCLUDE_DIRS}" )
    message ( STATUS "Python 3 libs: ${Python_LIBRARIES}" )
    message ( STATUS "Python 3 link options: ${Python_LINK_OPTIONS}" )
    include_directories (
        ${Python_INCLUDE_DIRS}
        ../../libtransform/
        )

    link_directories (
        ${Python_LIBRARY_DIRS}
        ${PROJECT_SOURCE_DIR}/../../lib
        )

    set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY 
        ${PROJECT_SOURCE_DIR}/../../bin/plugins 
        )

    set ( pythonplugin_HDRS
        pythonplugin_global.h
        pythontransform.h
        pythonmodules.h
        pythonplugin.h
        )

    set ( pythonplugin_SRCS
        pythonmodules.cpp
        pythontransform.cpp
        pythonplugin.cpp
        )

    set ( EXTERN_LIB transform )

    add_library ( python3plugin MODULE ${pythonplugin_SRCS} )

    target_link_libraries ( python3plugin
        PRIVATE Qt5::Widgets Qt5::Svg ${EXTERN_LIB}
        PUBLIC ${Python_LIBRARIES}
        )

    IF(UNIX)
        include( GNUInstallDirs )
        install (TARGETS python3plugin DESTINATION ${CMAKE_INSTALL_LIBDIR}/pip3line/
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    endif(UNIX)

endif()
