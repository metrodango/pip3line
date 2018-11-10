#!/bin/sh

mkdir -p Pip3line.app/Contents/

#install_name_tool -change libdistorm3.dylib @rpath/libdistorm3.dylib bin/plugins/libdistormplugin.so

cp -a bin Pip3line.app/Contents/MacOS

mkdir Pip3line.app/Contents/Resources

sips -s format icns gui/icons/pip3line.png --out Pip3line.app/Contents/Resources/pip3line.icns
# cp extras/macos/pip3line.icns Pip3line.app/Contents/Resources/

cp extras/macos/info.plist Pip3line.app/Contents/

