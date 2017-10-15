####
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information
####

message( STATUS "Configuring the Distorm plugin --")
project( distormplugin )

set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DDISTORM_LIBRARY )

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
	${DISTORM3_INCLUDE_PATH}
	../../libtransform
	../../../distorm/include
	/usr/include/distorm3
	)

link_directories (
	${DISTORM3_LIB_PATH}
	${PROJECT_SOURCE_DIR}/../../lib
	#	../../../distorm/build/lib/distorm3
	)

set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin/plugins )


if (WIN32)
	include_directories (
		${DISTORM3_INCLUDE_PATH}
		..\\..\\..\\distorm\\include
		)
	link_directories (
		${DISTORM3_LIB_PATH}
		..\\..\\..\\distorm
		)
elseif(APPLE)
	link_directories ( ../../../distorm/make/mac )
elseif(UNIX)
	#link_directories ( ../../../distorm/make/linux )
endif ()

set ( EXTERN_LIBS transform distorm3 )

set ( distormplugin_HDRS
	distorm_global.h
	distormplugin.h
	distormwidget.h
	distormtransf.h
	)

set ( distormplugin_SRCS
	distormplugin.cpp
	distormwidget.cpp
	distormtransf.cpp
	)
set ( distormplugin_UIS
	distormwidget.ui
	)

if(WITH_QT5)
       	qt5_wrap_ui(UIS ${distormplugin_UIS})
else(WITH_QT5)
       	QT4_WRAP_UI(UIS ${distormplugin_UIS})
endif(WITH_QT5)
	
add_library ( distormplugin MODULE ${distormplugin_SRCS} ${UIS} )
if(WITH_QT5)
       	qt5_use_modules(distormplugin Widgets Svg)
else(WITH_QT5)
	target_link_libraries ( distormplugin  ${QT_LIBRARIES} )
endif(WITH_QT5)

target_link_libraries ( distormplugin  ${EXTERN_LIBS} )

IF(UNIX)
	install (TARGETS distormplugin DESTINATION share/pip3line/
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
ENDIF(UNIX)


