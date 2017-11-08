####
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information
####

message( STATUS "Configuring the OpenSSL plugin --")
project( opensslplugin )

set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DOPENSSLPLUGIN_LIBRARY)

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
	../../libtransform/
	)

link_directories (
	${PROJECT_SOURCE_DIR}/../../lib
	)

set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin/plugins )

find_package(OpenSSL)

if (OPENSSL_FOUND)
	message( STATUS "OpenSSL library found ... OK")
	include_directories(${OPENSSL_INCLUDE_DIR})
	link_directories(${OPENSSL_LIBRARIES})
	if (UNIX)
		set ( EXTERN_LIBS transform crypto )
	endif (UNIX)

	if (WIN32)
		set ( EXTERN_LIBS transform libeay32 )
		if (CMAKE_SIZEOF_VOID_P MATCHES 8) #64bit
			link_directories (
				C:\\OpenSSL-Win64\\lib\\
				)
			include_directories (
				C:\\OpenSSL-Win64\\include\\
				)
		else(CMAKE_SIZEOF_VOID_P MATCHES 8) #32bit
				link_directories (
				C:\\OpenSSL-Win32\\lib\\
				)
			include_directories (
				C:\\OpenSSL-Win32\\include\\
				)
		endif(CMAKE_SIZEOF_VOID_P MATCHES 8)

	endif (WIN32)


	set ( opensslplugin_HDRS
		opensslplugin_global.h
		opensslplugin.h
		opensslhashes.h
		)

	set ( opensslplugin_SRCS
		opensslplugin.cpp
		opensslhashes.cpp
		)
	
	add_library ( opensslplugin MODULE ${opensslplugin_SRCS} )
	if(WITH_QT5)
        	qt5_use_modules(opensslplugin Widgets Svg)
	else(WITH_QT5)
		target_link_libraries ( opensslplugin  ${QT_LIBRARIES} )
	endif(WITH_QT5)

	target_link_libraries ( opensslplugin  ${EXTERN_LIBS} )

	IF(UNIX)
		install (TARGETS opensslplugin DESTINATION lib/pip3line/
			PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
			GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
	ENDIF(UNIX)

else (OPENSSL_FOUND)
	message( WARNING "OpenSSL library not found !! this plugin will not be compiled." )
endif (OPENSSL_FOUND)


