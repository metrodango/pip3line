####
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information
####

message ( STATUS "Configuring the example plugin --" )
project( exampleplugin )
set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DEXAMPLEPLUGIN_LIBRARY )


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
if(WITH_QT5)
        qt5_use_modules( exampleplugin Widgets Svg )
else(WITH_QT5)
	target_link_libraries ( exampleplugin ${QT_LIBRARIES} )
endif(WITH_QT5)

target_link_libraries ( exampleplugin ${EXTERN_LIB} )

IF(UNIX)
	install (TARGETS exampleplugin DESTINATION lib/pip3line/
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
ENDIF(UNIX)

