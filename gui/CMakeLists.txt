###
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information
###

cmake_minimum_required(VERSION 3.5)
message ( STATUS "Configuring pip3line gui --" )
project( pip3line )
#set ( CMAKE_BUILD_TYPE Release )

set(CMAKE_CXX_STANDARD 17)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../bin)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS shared)

if (WITH_SCINTILLA)
    set (QSCINTILLA_HOME "${PROJECT_SOURCE_DIR}\\..\\ext\\QScintilla_gpl")
    message ( STATUS "Enabling QScintilla support -- Warning QScintilla has its own drawbacks, not always the best choice" )
    add_definitions ( -DSCINTILLA -DQSCINTILLA_DLL)
    include_directories (
        ${QSCINTILLA_HOME}\\src /usr/local/include/
    )
    if (WIN32)
        link_directories (
             ${QSCINTILLA_HOME}\\src\\release
        )
    else()
        link_directories (
             ${QSCINTILLA_HOME}\\src\\release /usr/local/lib/
        )
    endif()
endif(WITH_SCINTILLA)

find_package(Qt5 COMPONENTS Core Widgets Svg Network Concurrent REQUIRED)

include_directories (
	../libtransform)

if (NOT APPLE)
  link_directories (
    ${PROJECT_SOURCE_DIR}/../lib
  )
else()
	message( STATUS "-- Enabling MAC OSX specifics --" )
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.perso.pip3line")
	set(APP_TYPE MACOSX_BUNDLE)
endif()

#set ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin )

set ( EXTERN_LIBS ${EXTERN_LIBS} transform )

if (WITH_SCINTILLA)
	if ((WIN32) OR (APPLE))
          set ( EXTERN_LIBS ${EXTERN_LIBS} qscintilla2_qt5)
	else()
          find_program(LSB_RELEASE lsb_release)
          execute_process(COMMAND ${LSB_RELEASE} -is
            OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
            OUTPUT_STRIP_TRAILING_WHITESPACE
          )
          execute_process(COMMAND ${LSB_RELEASE} -cs
            OUTPUT_VARIABLE LSB_RELEASE_CODENAME
            OUTPUT_STRIP_TRAILING_WHITESPACE
          )
          if (LSB_RELEASE_ID_SHORT STREQUAL "Ubuntu" AND LSB_RELEASE_CODENAME STREQUAL "xenial")
            set ( EXTERN_LIBS ${EXTERN_LIBS} qt5scintilla2)
          else ()
            set ( EXTERN_LIBS ${EXTERN_LIBS} qscintilla2_qt5)
          endif()
	endif()
endif(WITH_SCINTILLA)

if (UNIX)
	add_definitions ( -Wall )
endif (UNIX)

set ( pip3line_HDRS
	confgui/transformguimgmt.h
	mainwindow.h
	transformwidget.h
	aboutdialog.h
	messagedialog.h
	analysedialog.h
	massprocessingdialog.h
	regexphelpdialog.h
	loggerwidget.h
	infodialog.h
	screeniodevice.h
	settingsdialog.h
	downloadmanager.h
	pluginconfwidget.h
	guihelper.h
	maintabs.h
	floatingdialog.h
	quickviewdialog.h
	quickviewitem.h
	quickviewitemconfig.h
	newbytedialog.h
	debugdialog.h
	textinputdialog.h
	comparisondialog.h
	appdialog.h
        additionaluidialog.h
	../tools/processor.h
	../tools/pipeserver.h
	../tools/streamprocessor.h
	../tools/tcpserver.h
	../tools/textprocessor.h
	../tools/serverabstract.h
	../tools/processingstats.h
	../version.h
	sources/basicsource.h
	sources/bytesourceabstract.h
	sources/currentmemorysource.h
	sources/largerandomaccesssource.h
	sources/memorywidget.h
	sources/filewidget.h
	sources/largefile.h
	sources/searchabstract.h
	sources/intermediatesource.h
        sources/blocksources/blockssource.h
        sources/blocksources/fileblocksource.h
        sources/blocksources/filesourcewidget.h
	sources/blocksources/ipblockssources.h
	sources/blocksources/connectionsinfoswidget.h
	sources/blocksources/connectiondetails.h
	sources/blocksources/connectionswidget.h
	sources/blocksources/target.h
	sources/blocksources/udpclientlistener.h
	sources/blocksources/ipnetworkserverwidget.h
	sources/blocksources/udpserverlistener.h
	sources/blocksources/tlsserverlistener.h
	sources/blocksources/baseblocksourcewidget.h
	sources/blocksources/ipnetworkclientwidget.h
	sources/blocksources/tlsclientlistener.h
	sources/blocksources/socksproxyhelper.h
	sources/blocksources/pipeclientlistener.h
	sources/blocksources/pipeclientwidget.h
	sources/blocksources/sharedmemorysource.h
	sources/blocksources/shmtypes/shmsysvwidget.h
	sources/blocksources/shmtypes/sharedmemposixwidget.h
	sources/blocksources/shmtypes/sharedmemconnector.h
	sources/blocksources/shmtypes/sysv_shm.h
	sources/blocksources/shmtypes/sharedmemposix.h
	sources/blocksources/shmtypes/sharedmemorysourcebasewidget.h
        sources/blocksources/dtlsclientlistener.h
        sources/blocksources/dtlsserverlistener.h
        sources/blocksources/dtlscommon.h
        sources/blocksources/dtlsclientlistenerwidget.h
        sources/blocksources/dtlsserverlistenerwidget.h
	views/hexview.h
	views/textview.h
	views/bytetableview.h
	views/byteitemmodel.h
	views/singleviewabstract.h
	views/foldedview.h
	views/jsonview.h
	tabs/tababstract.h
	tabs/transformsgui.h
	tabs/generictab.h
	tabs/tababstract.h
	tabs/transformsgui.h
	tabs/randomaccesstab.h
	tabs/packetanalysertab.h
	tabs/packetanalyser/importexportdialog.h
	tabs/packetanalyser/importexportworker.h
	tabs/packetanalyser/memorypacketmodel.h
	tabs/packetanalyser/packetanalyseroptionsdialog.h
	tabs/packetanalyser/packetmodelabstract.h
	tabs/packetanalyser/packetsortfilterproxymodel.h
	tabs/packetanalyser/packetstyleditemdelegate.h
	tabs/packetanalyser/sourcesorchestatorabstract.h
	tabs/packetanalyser/orchestratorchooser.h
	tabs/packetanalyser/externalproxyorchestrator.h
	tabs/packetanalyser/proxyorchestrator.h
	tabs/packetanalyser/socksorchestrator.h
	tabs/packetanalyser/filterdialog.h
	tabs/packetanalyser/filteritem.h
	tabs/packetanalyser/filterengine.h
	tabs/packetanalyser/myoproxy.h
	tabs/packetanalyser/myoproxywidget.h
	shared/offsetgotowidget.h
	shared/searchwidget.h
	shared/readonlybutton.h
	shared/clearallmarkingsbutton.h
	shared/bytesourceguibutton.h
	shared/detachtabbutton.h
	shared/universalreceiverbutton.h
	shared/messagepanelwidget.h
	shared/searchresultswidget.h
	shared/newviewbutton.h
	shared/newviewmenu.h
	shared/guiconst.h
	shared/sslconf.h
	shared/ssloptionswidget.h
	shared/defaultcontrolgui.h
	shared/defaultdialog.h
	shared/sendtomenu.h
        shared/sendtobutton.h
        shared/targetswidget.h
	shared/transformguibutton.h
	shared/transformselectorwidget.h
	shared/newconnectionbutton.h
	state/basestateabstract.h
	state/stateorchestrator.h
	state/closingstate.h
	state/statedialog.h
	state/statestatuswidget.h
	pcapio/pcapio.h
	pcapio/pcapdef.h
	)


set ( pip3line_SRCS
	main.cpp
	win.rc
	mainwindow.cpp
	transformwidget.cpp
	messagedialog.cpp
	aboutdialog.cpp
	analysedialog.cpp
	massprocessingdialog.cpp
	regexphelpdialog.cpp
	loggerwidget.cpp
	infodialog.cpp
	screeniodevice.cpp
	settingsdialog.cpp
	downloadmanager.cpp
	pluginconfwidget.cpp
	guihelper.cpp
	maintabs.cpp
	floatingdialog.cpp
	quickviewdialog.cpp
	quickviewitem.cpp
	quickviewitemconfig.cpp
	newbytedialog.cpp
	debugdialog.cpp
	textinputdialog.cpp
	comparisondialog.cpp
	appdialog.cpp
        additionaluidialog.cpp
	../tools/processor.cpp
	../tools/textprocessor.cpp
	../tools/pipeserver.cpp
	../tools/streamprocessor.cpp
	../tools/tcpserver.cpp
	../tools/serverabstract.cpp
	../tools/processingstats.cpp
	sources/basicsource.cpp
	sources/bytesourceabstract.cpp
	sources/currentmemorysource.cpp
	sources/largerandomaccesssource.cpp
	sources/memorywidget.cpp
	sources/filewidget.cpp
	sources/largefile.cpp
	sources/searchabstract.cpp
	sources/intermediatesource.cpp
        sources/blocksources/blockssource.cpp
        sources/blocksources/fileblocksource.cpp
        sources/blocksources/filesourcewidget.cpp
	sources/blocksources/ipblockssources.cpp
	sources/blocksources/connectionsinfoswidget.cpp
	sources/blocksources/connectiondetails.cpp
	sources/blocksources/connectionswidget.cpp
	sources/blocksources/udpclientlistener.cpp
	sources/blocksources/ipnetworkserverwidget.cpp
	sources/blocksources/udpserverlistener.cpp
	sources/blocksources/tlsserverlistener.cpp
	sources/blocksources/baseblocksourcewidget.cpp
	sources/blocksources/ipnetworkclientwidget.cpp
	sources/blocksources/tlsclientlistener.cpp
	sources/blocksources/socksproxyhelper.cpp
	sources/blocksources/pipeclientlistener.cpp
	sources/blocksources/pipeclientwidget.cpp
	sources/blocksources/sharedmemorysource.cpp
	sources/blocksources/shmtypes/sharedmemconnector.cpp
	sources/blocksources/shmtypes/sharedmemorysourcebasewidget.cpp
	sources/blocksources/shmtypes/sharedmemposix.cpp
	sources/blocksources/shmtypes/sharedmemposixwidget.cpp
	sources/blocksources/shmtypes/sysv_shm.cpp
	sources/blocksources/shmtypes/shmsysvwidget.cpp
        sources/blocksources/dtlsclientlistener.cpp
        sources/blocksources/dtlsserverlistener.cpp
        sources/blocksources/dtlscommon.cpp
        sources/blocksources/dtlsclientlistenerwidget.cpp
        sources/blocksources/dtlsserverlistenerwidget.cpp
	views/hexview.cpp
	views/bytetableview.cpp
	views/byteitemmodel.cpp
	views/textview.cpp
	views/singleviewabstract.cpp
	views/foldedview.cpp
	views/jsonview.cpp
	tabs/generictab.cpp
	tabs/tababstract.cpp
	tabs/transformsgui.cpp
	tabs/transformsgui.cpp
	tabs/tababstract.cpp
	tabs/randomaccesstab.cpp
	tabs/packetanalysertab.cpp
	tabs/packetanalyser/importexportdialog.cpp
	tabs/packetanalyser/importexportworker.cpp
	tabs/packetanalyser/memorypacketmodel.cpp
	tabs/packetanalyser/packetanalyseroptionsdialog.cpp
	tabs/packetanalyser/packetmodelabstract.cpp
	tabs/packetanalyser/packetsortfilterproxymodel.cpp
	tabs/packetanalyser/packetstyleditemdelegate.cpp
	tabs/packetanalyser/sourcesorchestatorabstract.cpp
	tabs/packetanalyser/singlesourceorchestrator.cpp
	tabs/packetanalyser/orchestratorchooser.cpp
	tabs/packetanalyser/externalproxyorchestrator.cpp
	tabs/packetanalyser/proxyorchestrator.cpp
	tabs/packetanalyser/socksorchestrator.cpp
	tabs/packetanalyser/filterdialog.cpp
	tabs/packetanalyser/filteritem.cpp
	tabs/packetanalyser/filterengine.cpp
        tabs/packetanalyser/myoproxy.cpp
        tabs/packetanalyser/myoproxywidget.cpp
	shared/offsetgotowidget.cpp
	shared/searchwidget.cpp
	shared/readonlybutton.cpp
	shared/clearallmarkingsbutton.cpp
	shared/bytesourceguibutton.cpp
	shared/detachtabbutton.cpp
	shared/universalreceiverbutton.cpp
	shared/messagepanelwidget.cpp
	shared/searchresultswidget.cpp
	shared/newviewbutton.cpp
	shared/newviewmenu.cpp
	shared/guiconst.cpp
	shared/sslconf.cpp
	shared/ssloptionswidget.cpp
	shared/defaultcontrolgui.cpp
	shared/defaultdialog.cpp
	shared/sendtomenu.cpp
        shared/sendtobutton.cpp
        shared/targetswidget.cpp
	shared/transformselectorwidget.cpp
	shared/transformguibutton.cpp
	shared/newconnectionbutton.cpp
	state/basestateabstract.cpp
	state/stateorchestrator.cpp
	state/closingstate.cpp
	state/statedialog.cpp
	state/statestatuswidget.cpp
	pcapio/pcapdef.cpp
        pcapio/pcapio.cpp
)

set ( pip3line_UIS
	mainwindow.ui
	comparisondialog.ui
	transformwidget.ui
	aboutdialog.ui
	messagedialog.ui
	regexphelpdialog.ui
	analysedialog.ui
	loggerwidget.ui
	massprocessingdialog.ui
	insertdialog.ui
	infodialog.ui
	settingsdialog.ui
	pluginconfwidget.ui
        additionaluidialog.ui
	comparisondialog.ui
	textinputdialog.ui
	tabs/transformsgui.ui
	tabs/generictab.ui
	tabs/transformsgui.ui
	tabs/randomaccesstab.ui
	tabs/packetanalysertab.ui
	tabs/packetanalyser/importexportdialog.ui
	tabs/packetanalyser/packetanalyseroptionsdialog.ui
        tabs/packetanalyser/filterdialog.ui
        tabs/packetanalyser/myoproxywidget.ui
	floatingdialog.ui
	quickviewdialog.ui
	quickviewitem.ui
	quickviewitemconfig.ui
	newbytedialog.ui
	debugdialog.ui
	sources/memorywidget.ui
	sources/filewidget.ui
	sources/blocksources/connectionsinfoswidget.ui
	sources/blocksources/connectionswidget.ui
	sources/blocksources/filesourcewidget.ui
	sources/blocksources/ipnetworkserverwidget.ui
	sources/blocksources/baseblocksourcewidget.ui
	sources/blocksources/ipnetworkclientwidget.ui
	sources/blocksources/pipeclientwidget.ui
        sources/blocksources/dtlsclientlistenerwidget.ui
        sources/blocksources/dtlsserverlistenerwidget.ui
	sources/blocksources/shmtypes/shmsysvwidget.ui
	sources/blocksources/shmtypes/sharedmemposixwidget.ui
	sources/blocksources/shmtypes/sharedmemorysourcebasewidget.ui
	shared/searchresultswidget.ui
	shared/transformdisplayconf.ui
	shared/ssloptionswidget.ui
	shared/messagepanelwidget.ui
	shared/defaultcontrolgui.ui
	shared/defaultdialog.ui
        shared/targetswidget.ui
	shared/transformselectorwidget.ui
	state/statedialog.ui
	state/statestatuswidget.ui
	views/hexview.ui
	views/textview.ui
	views/foldedview.ui
)

set ( pip3line_RSCS
	gui_res.qrc
)

add_executable ( pip3line WIN32 ${pip3line_UIS} ${pip3line_SRCS} ${pip3line_RSCS})

if(WIN32)
    target_link_libraries ( pip3line Qt5::Widgets Qt5::Svg Qt5::Network Qt5::Concurrent Qt5::WinMain )
endif(WIN32)

target_link_libraries ( pip3line Qt5::Widgets Qt5::Svg Qt5::Network Qt5::Concurrent ${EXTERN_LIBS} )

if(UNIX)
	install (TARGETS pip3line DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
	if (NOT APPLE)
		install (FILES pip3line.desktop DESTINATION /usr/share/applications PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
		install (FILES icons/pip3line.png DESTINATION /usr/share/icons/hicolor/128x128/apps PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
	endif()
	include( GNUInstallDirs )
	install(FILES pip3line.1 DESTINATION ${CMAKE_INSTALL_FULL_MANDIR}/man1 COMPONENT doc)
endif(UNIX)

