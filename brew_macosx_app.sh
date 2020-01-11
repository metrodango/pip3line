#!/bin/sh

VERSION=`cat version.h | sed -n 's/#define VERSION_STRING "\([0-9.-]*\)"/\1/p'`

echo $VERSION

if [ -d Pip3line.app ] 
then
  rm -rf Pip3line.app;
fi 

mkdir -p Pip3line.app/Contents/

#install_name_tool -change libdistorm3.dylib @rpath/libdistorm3.dylib bin/plugins/libdistormplugin.so

#cp -a bin Pip3line.app/Contents/MacOS
ln -s /usr/local/Cellar/pip3line/${VERSION}/bin Pip3line.app/Contents/MacOS

mkdir Pip3line.app/Contents/Resources

sips -s format icns gui/icons/pip3line.png --out Pip3line.app/Contents/Resources/pip3line.icns

cat extras/macos/info.plist | sed "s/VERSION/${VERSION}/" > Pip3line.app/Contents/info.plist

