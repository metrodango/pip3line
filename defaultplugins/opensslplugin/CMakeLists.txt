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

find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Svg)

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
    set ( EXTERN_LIBS transform ${OPENSSL_CRYPTO_LIBRARY} )

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
    target_link_libraries ( opensslplugin Qt5::Widgets Qt5::Svg ${EXTERN_LIBS} )

    IF(UNIX)
        install (TARGETS opensslplugin DESTINATION lib/pip3line/
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
    ENDIF(UNIX)

else (OPENSSL_FOUND)
    message( WARNING "OpenSSL library not found !! this plugin will not be compiled." )
endif (OPENSSL_FOUND)


