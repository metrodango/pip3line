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

if(WITH_QT5)
	find_package(Qt5Widgets)
	find_package(Qt5Core)
	find_package(Qt5Svg)
	find_package(Qt5Concurrent)
else(WITH_QT5)	
	find_package ( Qt4 REQUIRED QtCore QtGui QtSvg)
	include ( ${QT_USE_FILE} )
endif(WITH_QT5)

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

if(WITH_QT5)
	qt5_wrap_ui(UIS ${transform_UIS})
else(WITH_QT5)
	qt4_wrap_ui(UIS ${transform_UIS})
endif(WITH_QT5)


add_library ( transform SHARED ${transform_SRCS} ${RSCS} ${UIS} )

if (UNIX)
	set_target_properties(transform PROPERTIES VERSION ${LIBTRANSFORM_VERSION_MAJOR}
		SOVERSION ${LIBTRANSFORM_VERSION_MAJOR})
endif(UNIX)
if(WITH_QT5)
	qt5_use_modules(transform Widgets Svg Concurrent)
else(WITH_QT5)
	target_link_libraries ( transform  ${QT_LIBRARIES} )
endif(WITH_QT5)

IF(UNIX)
	install (TARGETS transform DESTINATION lib)
	install (FILES ${transform_HDRS} DESTINATION include/libtransform )
	install (FILES ${transform_MOCS} DESTINATION include/libtransform )
	install (FILES LibTransform DESTINATION include )
ENDIF(UNIX)

