###
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under BSD 3-Clause see LICENSE for more information
###

cmake_minimum_required(VERSION 3.5)
message( STATUS "Configuring libtransform --" )
project( transform )
#set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DLIBTRANSFORM_LIBRARY)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(STRINGS commonstrings.h VERSION_FILE)

STRING(REGEX MATCH ".* LIB_TRANSFORM_VERSION ([0-9]+);.*" OUPUT_MATCH "${VERSION_FILE}")


if ("${CMAKE_MATCH_1}" STREQUAL "")
    message( FATAL_ERROR " Cannot determine LibTransform version number" )
else()
    set (LIBTRANSFORM_VERSION_MAJOR ${CMAKE_MATCH_1})
    message( STATUS "Lib transform version : ${LIBTRANSFORM_VERSION_MAJOR}")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Svg Concurrent)

set ( transform_HDRS
    commonstrings.h
    composedtransform.h
    deleteablelistitem.h
    hexwidget.h
    libtransform_global.h
    modulesmanagement.h
    modulesmanagementwidget.h
    moduletransformwidget.h
    pip3linecallback.h
    scripttransformabstract.h
    threadedprocessor.h
    transformabstract.h
    transformchain.h
    transformfactoryplugininterface.h
    transformmgmt.h
    packethandler.h
    pipelinecommon.h
    )

set ( transform_SRCS
    commonstrings.cpp
    composedtransform.cpp
    deleteablelistitem.cpp
    hexwidget.cpp
    modulesmanagement.cpp
    modulesmanagementwidget.cpp
    moduletransformwidget.cpp
    pip3linecallback.cpp
    scripttransformabstract.cpp
    threadedprocessor.cpp
    transformabstract.cpp
    transformchain.cpp
    transformmgmt.cpp
    packethandler.cpp
    pipelinecommon.cpp
    )

set ( transform_UIS
    deleteablelistitem.ui
    hexwidget.ui
    modulesmanagementwidget.ui
    moduletransformwidget.ui
    )

add_library ( transform SHARED ${transform_SRCS} ${RSCS} ${transform_UIS} )

if (UNIX)
    set_target_properties(transform PROPERTIES VERSION ${LIBTRANSFORM_VERSION_MAJOR}
        SOVERSION ${LIBTRANSFORM_VERSION_MAJOR})
    set_target_properties(transform PROPERTIES PUBLIC_HEADER
            "${transform_HDRS};${transform_MOCS};LibTransform")
endif(UNIX)

target_link_libraries(transform PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg Qt${QT_VERSION_MAJOR}::Concurrent)

IF(UNIX)
        include( GNUInstallDirs )
        install (TARGETS transform LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                PUBLIC_HEADER DESTINATION include/libtransform )
ENDIF(UNIX)

