#!/bin/sh

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     basePath=distorm/make/linux;;
    Darwin*)    basePath=distorm/make/mac;;
    *)          echo "Unamanaged platform: ${unameOut}" || exit -1;
esac

echo "Entering ${basePath}"

cd ${basePath}

make
