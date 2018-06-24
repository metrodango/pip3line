####
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information
####

message( STATUS "Configuring libtransform --" )
project( transform )
set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DLIBTRANSFORM_LIBRARY)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(STRINGS commonstrings.h VERSION_FILE)

STRING(REGEX MATCH ".* LIB_TRANSFORM_VERSION ([0-9]+);.*" OUPUT_MATCH
	"${VERSION_FILE}")


if ("${CMAKE_MATCH_1}" STREQUAL "")
	message( FATAL_ERROR " Cannot determine LibTransform version number" )
else()
	set (LIBTRANSFORM_VERSION_MAJOR ${CMAKE_MATCH_1})
	message( STATUS "Lib transform version : ${LIBTRANSFORM_VERSION_MAJOR}")
endif()

find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Svg)
find_package(Qt5Concurrent)

set ( transform_HDRS
	transformfactoryplugininterface.h
	commonstrings.h
	transformmgmt.h
	transformabstract.h
	pip3linecallback.h
	transformchain.h
	composedtransform.h
	modulesmanagement.h
	modulesmanagementwidget.h
	moduletransformwidget.h
	scripttransformabstract.h
	deleteablelistitem.h
	threadedprocessor.h
	hexwidget.h
	)

set ( transform_SRCS
	transformmgmt.cpp
	transformabstract.cpp
	pip3linecallback.cpp
	transformchain.cpp
	commonstrings.cpp
	composedtransform.cpp
	modulesmanagement.cpp
	modulesmanagementwidget.cpp
	moduletransformwidget.cpp
	scripttransformabstract.cpp
	deleteablelistitem.cpp
	threadedprocessor.cpp
	hexwidget.cpp
	)

set ( transform_UIS
	modulesmanagementwidget.ui
	moduletransformwidget.ui
	deleteablelistitem.ui
	hexwidget.ui
	)

qt5_wrap_ui(UIS ${transform_UIS})

add_library ( transform SHARED ${transform_SRCS} ${RSCS} ${UIS} )

if (UNIX)
	set_target_properties(transform PROPERTIES VERSION ${LIBTRANSFORM_VERSION_MAJOR}
		SOVERSION ${LIBTRANSFORM_VERSION_MAJOR})
endif(UNIX)

target_link_libraries(transform Qt5::Widgets Qt5::Svg Qt5::Concurrent)

IF(UNIX)
	install (TARGETS transform DESTINATION lib)
	install (FILES ${transform_HDRS} DESTINATION include/libtransform )
	install (FILES ${transform_MOCS} DESTINATION include/libtransform )
	install (FILES LibTransform DESTINATION include )
ENDIF(UNIX)

