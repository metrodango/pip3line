####
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information
####

if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.0)
    cmake_minimum_required( VERSION 3.1 )
else ()
    cmake_minimum_required( VERSION 2.8.11 )
endif()

message( STATUS "Accepted Parameters" )
message( STATUS "* ALL : enable all plugins (fit for the target platform)" )
message( STATUS "* WITH_PYTHON27 : enable/disable the Python2.7 plugin" )
message( STATUS "    PYTHON27_INCLUDE_DIRS : specify the include path for the Python 2.7 headers" )
message( STATUS "    PYTHON27_LIBRARIES : specify the lib path for the Python 2.7 shared lib" )
message( STATUS "* WITH_PYTHON3 : enable/disable the Python3 plugin" )
message( STATUS "    PYTHON3_INCLUDE_DIRS : specify the include path for the Python 3.x headers" )
message( STATUS "    PYTHON3_LIBRARIES : specify the lib path for the Python 3.x shared lib" )
message( STATUS "* WITH_OPENSSL : enable/disable the Openssl plugin" )
message( STATUS "* WITH_DISTORM : enable/disable the Distorm plugin" )
message( STATUS "    WITH_DISTORM_LINK_STATICALLY : Fetch and build a static version (Unix-like target only)" )
message( STATUS "* WITH_SCINTILLA : use QScintilla for more efficient text rendering" )
message( STATUS "* ENABLE_ADDR_SANITIZER : Compile with asan (gcc/clang with asan only)" )
message( STATUS "Example:" )
message( STATUS "cmake .. -DALL=ON -DWITH_PYTHON27=OFF" )
message( STATUS "---" )

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    message( STATUS "Git information --" )
    execute_process(COMMAND ${GIT_EXECUTABLE} -C ${CMAKE_CURRENT_SOURCE_DIR} branch OUTPUT_VARIABLE GIT_BRANCH)
    STRING(REGEX REPLACE ".*\\* ([a-zA-Z0-9]+)\n.*" "\\1" GIT_BRANCH "${GIT_BRANCH}")
    STRING(REGEX REPLACE "\n" "" GIT_BRANCH "${GIT_BRANCH}")
    
    if (GIT_BRANCH STREQUAL "")
        SET(GIT_BRANCH "Unknown")
    endif()
    message(STATUS "Git branch: ${GIT_BRANCH}")
    
    execute_process(COMMAND ${GIT_EXECUTABLE} -C ${CMAKE_CURRENT_SOURCE_DIR} rev-parse HEAD OUTPUT_VARIABLE GIT_COMMIT)

    STRING(REGEX REPLACE "(\r?\n)+$" "" GIT_COMMIT "${GIT_COMMIT}")
    if (GIT_COMMIT STREQUAL "")
        SET(GIT_COMMIT "Unknown")
    endif()
    message(STATUS "Building using git commit: ${GIT_COMMIT}")
    
else()
    message( STATUS "Missing git binary or directory, Unknown git version --" )
    SET(GIT_COMMIT "Unknown")
    SET(GIT_BRANCH "Unknown")
endif()

add_definitions(-DGIT_BRANCH=\"${GIT_BRANCH}\")
add_definitions(-DGIT_COMMIT=\"${GIT_COMMIT}\")

message( STATUS "General Configuration --" )

project( pip3lineall )
set ( CMAKE_BUILD_TYPE Release )

add_definitions(-DQT_NO_DEBUG_OUTPUT)

set (CMAKE_CXX_STANDARD 14)

if (MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP ")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP ")
    message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
    message(STATUS "CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")
endif(MSVC)

if (CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?Clang$")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wformat-security -Wextra")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wformat-security -Wextra")
endif()

if (ENABLE_ADDR_SANITIZER STREQUAL "ON")
  if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fpermissive \
    -Wall -Wformat-security -lasan -O -g -fno-omit-frame-pointer")
    message(STATUS "The address sanitizer was enabled for gcc.")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.8)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
    endif()
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fpermissive \
          -O -g -fno-omit-frame-pointer")
    set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -fsanitize=address")
    message(STATUS "The address sanitizer was enabled for clang.")
  endif()
    message(STATUS "Use ASAN_OPTIONS=symbolize=1 ASAN_SYMBOLIZER_PATH=$(which llvm-symbolizer) ./pip3line")
endif()

if (CMAKE_COMPILER_IS_GNUCC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s -Wall -Wformat-security -Wextra -Wpedantic")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s -Wall -Wformat-security -Wextra -Wpedantic")
    add_definitions( -DQT_DEPRECATED_WARNINGS=1)
endif()

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

if (WIN32)
    set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
else (WIN32)
    set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
endif(WIN32)

include_directories ( ${PROJECT_SOURCE_DIR}/libtransform )

if (WIN32)
    link_directories (
        ${project_source_dir}/lib
        ${project_source_dir}/bin
        ${project_source_dir}/bin/plugins
    )
else()
    link_directories (
        ${project_source_dir}/bin
        ${project_source_dir}/bin/plugins
    )
endif()

if (ALL)
    message ( STATUS  "Selecting all packages")
    SET (WITH_LIB ON)
    SET (WITH_GUI ON)
    SET (WITH_CMD ON)

    if (WITH_PYTHON3 STREQUAL "OFF")
        message(WARNING "The Python 3.x plugin has been manually disabled")
    else()
        SET (WITH_PYTHON3 ON)
    endif()

    if (WITH_PYTHON27 STREQUAL "OFF")
        message(WARNING "The Python 2.7 plugin has been manually disabled")
    else()
        SET (WITH_PYTHON27 ON)
    endif()

    if (WITH_OPENSSL STREQUAL "OFF")
        message(WARNING "The Openssl plugin has been manually disabled")
    else()
        SET (WITH_OPENSSL ON)
    endif()

    if (WITH_DISTORM STREQUAL "OFF")
        message(WARNING "The Distorm plugin has been manually disabled")
    else()
        SET (WITH_DISTORM ON)
    endif()

    if (WITH_EXAMPLE STREQUAL "OFF")
        message(WARNING "The example plugin has been manually disabled")
    else()
        SET (WITH_EXAMPLE ON)
    endif()

elseif ( BASIC )
    message ( STATUS "Selecting only base packages ")
    SET (WITH_LIB ON)
    SET (WITH_GUI ON)
    SET (WITH_CMD ON)
endif ()

if (WITH_LIB OR WITH_GUI OR WITH_CMD)
    add_subdirectory(libtransform)
    add_subdirectory(defaultplugins/baseplugins)
endif ()

if ( WITH_GUI )
    add_subdirectory(gui)
endif ( WITH_GUI )

if ( WITH_CMD )
    add_subdirectory(cmdline)
endif( WITH_CMD )

if ( WITH_OPENSSL )
    add_subdirectory(defaultplugins/opensslplugin)
endif ( WITH_OPENSSL )

if ( WITH_PYTHON3 ) 
    add_subdirectory(defaultplugins/pythonplugin)
endif ( WITH_PYTHON3 )

if ( WITH_PYTHON27 )
    if (APPLE AND WITH_PYTHON3)
        message(WARNING "Can't have both Python 2.7 and Python 3 plugins on OS X, disabling Python 2.7 plugin")
    else ()
        add_subdirectory(defaultplugins/python27plugin)
    endif ()
endif ( WITH_PYTHON27 )

if ( WITH_DISTORM ) 
    add_subdirectory(defaultplugins/distormplugin)
endif ( WITH_DISTORM )

if ( WITH_EXAMPLE ) 
    add_subdirectory(defaultplugins/exampleplugin)
endif ( WITH_EXAMPLE )

if (UNIX)
    # uninstall target
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif (UNIX)
