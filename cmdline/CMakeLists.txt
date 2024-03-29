#
# pip3line: easy encoder/decoder and more
# Copyright (C) 2012-2013  Gabriel Caudrelier<gabriel.caudrelier@gmail.com>
#
# Pip3line is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Pip3line is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Pip3line.  If not, see <http://www.gnu.org/licenses/>.
#

message ( STATUS "Configuring pip3linecmd --" )
project( pip3linecmd )
# set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Svg)

include_directories (
    ../libtransform
)

link_directories (
    ${PROJECT_SOURCE_DIR}/../lib
    )

set ( EXTERN_LIBS transform)

set ( pip3linecmd_HDRS
    ../tools/binaryprocessor.h
    ../tools/processingstats.h
    ../tools/processor.h
    ../tools/serverabstract.h
    ../tools/textprocessor.h
    ../version.h
    masterthread.h
    )

set ( pip3linecmd_SRCS
    ../tools/binaryprocessor.cpp
    ../tools/processingstats.cpp
    ../tools/processor.cpp
    ../tools/serverabstract.cpp
    ../tools/textprocessor.cpp
    main.cpp
    masterthread.cpp
    )

add_executable ( pip3linecmd ${pip3linecmd_SRCS} )

if(WIN32)
    target_link_libraries ( pip3linecmd PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg Qt${QT_VERSION_MAJOR}::WinMain ${EXTERN_LIBS} )
else(WIN32)
    target_link_libraries ( pip3linecmd PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg ${EXTERN_LIBS} )
endif(WIN32)

if(UNIX)
    include( GNUInstallDirs )
    install (TARGETS pip3linecmd DESTINATION bin)
    install(FILES pip3linecmd.1
       DESTINATION ${CMAKE_INSTALL_FULL_MANDIR}/man1 COMPONENT doc)
endif(UNIX)
