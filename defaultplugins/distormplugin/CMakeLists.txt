###
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under BSD 3-Clause see LICENSE for more information
###

message( STATUS "Configuring the Distorm plugin --")

project( distormplugin )

# set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall -DDISTORM_LIBRARY )

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Svg)

include_directories (
    ${DISTORM3_INCLUDE_PATH}
    ${PROJECT_SOURCE_DIR}/../../libtransform
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
        ${PROJECT_SOURCE_DIR}\\..\\..\\ext\\distorm\\include
    )
    link_directories (
        ${DISTORM3_LIB_PATH}
        ${PROJECT_SOURCE_DIR}\\..\\..\\ext\\distorm
    )
elseif(APPLE)
    include_directories ( ${PROJECT_SOURCE_DIR}/../../ext/distorm/include)
    link_directories ( ${PROJECT_SOURCE_DIR}/../../ext/distorm/make/mac )
elseif(UNIX)
    include_directories ( ${PROJECT_SOURCE_DIR}/../../ext/distorm/include)
    link_directories ( ${PROJECT_SOURCE_DIR}/../../ext/distorm/make/linux )
endif ()

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

add_library ( distormplugin MODULE ${distormplugin_SRCS} ${distormplugin_UIS} )
if(UNIX AND WITH_DISTORM_LINK_STATICALLY)
        find_package(Git QUIET)
        if(GIT_FOUND AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../ext/distorm/make")
                message(STATUS "fetching distorm")
                if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../.gitmodules" AND
                    EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../.git" )
                        execute_process(COMMAND ${GIT_EXECUTABLE} -C ${CMAKE_CURRENT_SOURCE_DIR}/../.. submodule update --init --recursive
                            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                            RESULT_VARIABLE GIT_SUBMOD_RESULT)
                        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
                                message(FATAL_ERROR "git submodule update failed: ${GIT_SUBMOD_RESULT}")
                        endif()
                else()
                        execute_process(COMMAND ${GIT_EXECUTABLE} clone https://github.com/gdabah/distorm.git 
                                ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/distorm
                                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                                RESULT_VARIABLE GIT_CLONE_RESULT)
                        if(NOT GIT_CLONE_RESULT EQUAL "0")
                                message(FATAL_ERROR "git clone failed: ${GIT_CLONE_RESULT}")
                        endif()
                endif()
        endif()
        if (APPLE)
                set ( OS_PATH "mac" )
                execute_process ( COMMAND sed -i -E "s/-arch i386//" ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/distorm/make/mac/Makefile )
                set(ENV{DISTORM_MODE} "static")
                execute_process ( COMMAND sed -i -E "s/#ar rs/ar rs/" ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/distorm/make/mac/Makefile )
        else()
                set ( OS_PATH "linux" )
        endif()
        message(STATUS "${OS_PATH} build / ${CMAKE_GENERATOR}")
        if (${CMAKE_GENERATOR} MATCHES "Ninja")
            message(STATUS "Ninja generator")
            add_custom_target(build_distorm_lib ALL
                    COMMAND make clib
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/distorm/make/${OS_PATH}
                    COMMENT "distorm make file")
        else()
            message(STATUS "Other generator")
            add_custom_target(build_distorm_lib ALL
                    COMMAND "$(MAKE)" clib
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/distorm/make/${OS_PATH}
                    COMMENT "distorm make file")
        endif()
        add_library (distorm3 STATIC IMPORTED)
        SET_TARGET_PROPERTIES(distorm3 PROPERTIES
                IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../../ext/distorm/distorm3.a)
        add_dependencies(distormplugin build_distorm_lib)
endif ()
set ( EXTERN_LIBS transform distorm3 )

target_link_libraries ( distormplugin PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg ${EXTERN_LIBS} )

IF(UNIX)
    include( GNUInstallDirs )
    install (TARGETS distormplugin DESTINATION ${CMAKE_INSTALL_LIBDIR}/pip3line/
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
ENDIF(UNIX)


