####
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under BSD 3-Clause see LICENSE for more information
####

message ( STATUS "Configuring the Python 2.7 plugin --" )
project( python27plugin )

set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DPYTHONPLUGIN_LIBRARY -DQT_NO_KEYWORDS)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Core Widgets Svg REQUIRED)

# hardcoded paths, in case none are supplied or found

if (WIN32)
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set (PYTHON27_HARDCODED_INCLUDE "C:\\Python27\\include")
        set (PYTHON27_HARDCODED_LIBS "C:\\Python27\\libs\\python27.lib")
    else ()
        set (PYTHON27_HARDCODED_INCLUDE "C:\\Python27-x64\\include")
        set (PYTHON27_HARDCODED_LIBS "C:\\Python27-x64\\libs")
    endif()
elseif(APPLE)
    set (PYTHON27_HARDCODED_INCLUDE /System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7)
    set (PYTHON27_HARDCODED_LIBS /System/Library/Frameworks/Python.framework/Versions/2.7/lib/libpython2.7.dylib) 
else()
    include( GNUInstallDirs )
    set (PYTHON27_HARDCODED_INCLUDE "/usr/include/python2.7")
    set (PYTHON27_HARDCODED_LIBS "/usr/${CMAKE_INSTALL_LIBDIR}/libpython2.7.so")
    set (PYTHON27_HARDCODED_LIBS_UBUNTU "/usr/lib/x86_64-linux-gnu/libpython2.7.so")
endif()

if (NOT PYTHON27_INCLUDE_DIRS OR NOT PYTHON27_LIBRARIES)
    message ( STATUS "Python 2.7 paths haven't been found or supplied, trying hardcoded values" )
    if ( EXISTS ${PYTHON27_HARDCODED_INCLUDE})
        set (PYTHON_INCLUDE_DIRS ${PYTHON27_HARDCODED_INCLUDE})
    else()
        message ( FATAL_ERROR "No Python 2.7 headers found" )
    endif()

    if ( EXISTS ${PYTHON27_HARDCODED_LIBS})
        set (PYTHON_LIBRARIES ${PYTHON27_HARDCODED_LIBS})
    else()
        if ( EXISTS ${PYTHON27_HARDCODED_LIBS_UBUNTU} )
            message ( STATUS "Found them: ${PYTHON27_HARDCODED_LIBS_UBUNTU}")
            set (PYTHON_LIBRARIES ${PYTHON27_HARDCODED_LIBS_UBUNTU} )
        else()
            message ( FATAL_ERROR "No Python 2.7 libraries found, Even hardcoded ones" )
        endif()
    endif()
else()
    message ( STATUS "Python 2.7 include & libs paths has been supplied by user" )
    set (PYTHON_INCLUDE_DIRS  ${PYTHON27_INCLUDE_DIRS})
    set (PYTHON_LIBRARIES ${PYTHON27_LIBRARIES})
endif()

if (PYTHON_INCLUDE_DIRS AND PYTHON_LIBRARIES)
    message ( STATUS "Python 2.7 include: ${PYTHON_INCLUDE_DIRS}" )
    message ( STATUS "Python 2.7 libs: ${PYTHON_LIBRARIES}" )
    
    include_directories (
        SYSTEM
        ${PYTHON_INCLUDE_DIRS}
        )

    include_directories (
        ../../libtransform/)

    link_directories (
        ${PYTHON_LIBRARIES}
        ${PROJECT_SOURCE_DIR}/../../lib
        )

    set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin/plugins )

    set ( pythonplugin_HDRS
        ../pythonplugin/pythonplugin_global.h
        ../pythonplugin/pythontransform.h
        ../pythonplugin/pythonplugin.h
        ../pythonplugin/pythonmodules.h
        )

    set ( pythonplugin_SRCS
        ../pythonplugin/pythontransform.cpp
        ../pythonplugin/pythonplugin.cpp
        ../pythonplugin/pythonmodules.cpp
        )

    if(WIN32)
        set ( EXTERN_LIB transform python27 )
    else(WIN32)
        set ( EXTERN_LIB transform python2.7 )
    endif(WIN32)

    add_library ( python27plugin MODULE ${pythonplugin_SRCS} )

    target_link_libraries ( python27plugin Qt5::Widgets Qt5::Svg ${EXTERN_LIB} )

    IF(UNIX)
        include( GNUInstallDirs )
        install (TARGETS python27plugin DESTINATION ${CMAKE_INSTALL_LIBDIR}/pip3line/
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    endif(UNIX)
endif()
