####
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information
####

message ( STATUS "Configuring the Python 2.7 plugin --" )
project( python27plugin )

set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DPYTHONPLUGIN_LIBRARY -DQT_NO_KEYWORDS)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WITH_QT5)
        find_package(Qt5Widgets)
        find_package(Qt5Core)
        find_package(Qt5Svg)
else(WITH_QT5)
        find_package ( Qt4 REQUIRED QtCore QtGui QtSvg)
	include ( ${QT_USE_FILE} )
endif(WITH_QT5)

# hardcoded paths, in case none are supplied or found

if (WIN32)
    set (PYTHON27_HARDCODED_INCLUDE C:\\Python27\\include)
    set (PYTHON27_HARDCODED_LIBS C:\\Python27\\libs\\)
elseif(APPLE)
    set (PYTHON27_HARDCODED_INCLUDE /Library/Frameworks/Python.framework/Versions/2.7/Headers/)
    set (PYTHON27_HARDCODED_LIBS /Library/Frameworks/Python.framework/Versions/2.7/lib/)
else()
    set (PYTHON27_HARDCODED_INCLUDE "/usr/include/python2.7")
    set (PYTHON27_HARDCODED_LIBS "/usr/lib/libpython2.7.so")
    set (PYTHON27_HARDCODED_LIBS_UBUNTU
	  "/usr/lib/x86_64-linux-gnu/libpython2.7.so")
endif()

if (NOT PYTHON27_INCLUDE_DIRS OR NOT PYTHON27_LIBRARIES)
    message ( WARNING "Python 2.7 paths haven't been found or supplied, trying hardcoded values" )
    if ( EXISTS ${PYTHON27_HARDCODED_INCLUDE})
        set (PYTHON_INCLUDE_DIRS ${PYTHON27_HARDCODED_INCLUDE})
    else()
        message ( FATAL_ERROR "No Python 2.7 include found" )
    endif()

    if ( EXISTS ${PYTHON27_HARDCODED_LIBS})
        set (PYTHON_LIBRARIES ${PYTHON27_HARDCODED_LIBS})
    else()
        if ( EXISTS ${PYTHON27_HARDCODED_LIBS_UBUNTU} )
            set (PYTHON_LIBRARIES ${PYTHON27_HARDCODED_LIBS_UBUNTU} )
        else()
            message ( FATAL_ERROR "No Python 2.7 libraries found, Even hardcoded ones" )
        endif()
    endif()
else()
    set (PYTHON_INCLUDE_DIRS  ${PYTHON27_INCLUDE_DIRS})
    set (PYTHON_LIBRARIES ${PYTHON27_LIBRARIES})
endif()


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

if(WITH_QT5)
	qt5_use_modules( python27plugin  Widgets Svg )
else(WITH_QT5)
	target_link_libraries ( python27plugin ${QT_LIBRARIES} )
endif(WITH_QT5)

target_link_libraries ( python27plugin ${EXTERN_LIB} )


IF(UNIX)
	install (TARGETS python27plugin DESTINATION share/pip3line/
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
ENDIF(UNIX)
