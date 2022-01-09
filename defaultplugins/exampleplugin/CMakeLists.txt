###
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information
###

message ( STATUS "Configuring the example plugin --" )
project( exampleplugin )
#set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DEXAMPLEPLUGIN_LIBRARY )

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Svg)

include_directories (
    ../../libtransform/)

link_directories (
    ${PROJECT_SOURCE_DIR}/../../lib
    )

set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin/plugins )

set ( exampleplugin_HDRS
    exampleplugin_global.h
    exampleplugin.h
    identity.h
    )

set ( exampleplugin_SRCS
    exampleplugin.cpp
    identity.cpp
    )

set ( EXTERN_LIB transform )

add_library ( exampleplugin MODULE ${exampleplugin_SRCS} )

target_link_libraries ( exampleplugin PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg ${EXTERN_LIB} )


IF(UNIX)
    include( GNUInstallDirs )
    install (TARGETS exampleplugin DESTINATION ${CMAKE_INSTALL_LIBDIR}/pip3line/
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )
ENDIF(UNIX)

