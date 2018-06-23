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

find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Svg)

include_directories (
	${DISTORM3_INCLUDE_PATH}
	../../libtransform
	/usr/include/distorm3
	/usr/lib/include/distorm3
	)

link_directories (
	${DISTORM3_LIB_PATH}
	${PROJECT_SOURCE_DIR}/../../lib
	)

set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin/plugins )


if (WIN32)
	include_directories (
		${DISTORM3_INCLUDE_PATH}
		..\\..\\ext\\distorm\\include
		)
	link_directories (
		${DISTORM3_LIB_PATH}
		..\\..\\ext\\distorm
		)
elseif(APPLE)
        include_directories ( ../../ext/distorm/include)
	link_directories ( ../../ext/distorm/make/mac )
elseif(UNIX)
        include_directories ( ../../ext/distorm/include)
	link_directories ( ../../ext/distorm/make/linux )
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

qt5_wrap_ui(UIS ${distormplugin_UIS})
	
add_library ( distormplugin MODULE ${distormplugin_SRCS} ${UIS} )

qt5_use_modules(distormplugin Widgets Svg)

target_link_libraries ( distormplugin  ${EXTERN_LIBS} )

IF(UNIX)
	install (TARGETS distormplugin DESTINATION lib/pip3line/
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
		GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
ENDIF(UNIX)


