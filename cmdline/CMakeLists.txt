####
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
####

message ( STATUS "Configuring pip3linecmd --" )
project( pip3linecmd )
set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall )

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WITH_QT5)
        find_package(Qt5Widgets)
        find_package(Qt5Core)
        find_package(Qt5Svg)
else(WITH_QT5)
	find_package ( Qt4 REQUIRED QtCore QtGui QtSvg QtMain)
	include ( ${QT_USE_FILE} )
endif(WITH_QT5)

include_directories (
	../libtransform
	)

link_directories (
	${PROJECT_SOURCE_DIR}/../lib
	)

set ( EXTERN_LIBS transform)

set ( pip3linecmd_HDRS
	masterthread.h
	../tools/processor.h
	../version.h
	../tools/binaryprocessor.h
	../tools/textprocessor.h
	../tools/serverabstract.h
	../tools/processingstats.h
	)

set ( pip3linecmd_SRCS
	main.cpp
	masterthread.cpp
	../tools/processor.cpp
	../tools/binaryprocessor.cpp
	../tools/textprocessor.cpp
	../tools/serverabstract.cpp
	../tools/processingstats.cpp
	)

add_executable ( pip3linecmd ${pip3linecmd_SRCS} )

if(WITH_QT5)
	qt5_use_modules(pip3linecmd Widgets Svg)
	if(WIN32)		
		target_link_libraries ( pip3linecmd  Qt5::WinMain)
	endif(WIN32)
else(WITH_QT5)
	target_link_libraries ( pip3linecmd  ${QT_LIBRARIES} )
endif(WITH_QT5)

target_link_libraries ( pip3linecmd  ${EXTERN_LIBS} )

if(UNIX)
	install (TARGETS pip3linecmd DESTINATION bin)
endif(UNIX)
