# Build process

## Requirements

 * A working QT5 dev environment
 * A working cmake environment
 * OpenSSL development libraries and headers (optional)
 * Python 2.7 or 3 (or both) dev environment (optional)
 * Distorm development libraries and headers (optional)

Qt4 is not supported anymore, too many changes have occurred , and Pip3line is now using Qt5 new APIs.

This project is based on the Qt library which is multi-platform, so the building process should roughly be the same once you have a working dev environment.

You also need cmake for an easier build.
See http://www.cmake.org/

## Generic compilation options

> -DALL=ON

Configure and compile everything.
In this configuration non-basic modules can be deactivated individually by setting them to OFF

> -DBASIC=ON

Configure and compile only the bare necessary parts:
* libtransform
* The Pip3line GUI
* The command line tool
* The default transformations plugin

## Linux build & Install

The Linux build and installation process is pretty much painless

> shell$ mkdir build

> shell$ cd build

> shell$ cmake [options] ../ (Or whatever is the path for the sources)

> shell$ make

If you want to speed up the compilation process on an eight cores machine you can replace the last step with:

> shell$ make -j8

Then installation as follow:

> shell$ sudo make install

Be careful that the the base install path is "/usr/local" by default.
If your library path is not configured to check /usr/local/lib this may cause the issue:
```
pip3line: error while loading shared libraries: libtransform.so: cannot open shared object file: No such file or directory
```

If you want/need a different path, for example "/usr" you may run cmake with the following option

> shell$ cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ../ 

There is also an uninstall _make_ command available

> shell$ sudo make uninstall

### Ubuntu

Make sure to install the following package:

python2.7-dev
python3-dev
libqt5svg5-dev
qt5-default
libqt5xmlpatterns5-dev
cmake
git
libssl-dev
libdistorm3-dev

## Windows specific
Brace yourself, the build process on a Windows platform can be quite tricky.

First, use the preconfigured Qt command lines shortcuts to start with a proper pre-configured environment. Additionaly this guide only covers building with Visual Studio (2013 and onwards)

On windows you need to specify the nmake generator for CMake.

You also have to run the script vsvars.bat located in the Visual Studio directory first.

For an x64 compilation with VS 2015:

> C:\path> "c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

For an x86 compilation with VS 2015:

> C:\path> "c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

Then simply:
> C:\path> cmake -G  "NMake Makefiles" ..\ [ + options]

for example:
> C:\path> cmake -G  "NMake Makefiles" ..\  -DALL=ON

Following with the compilation phase
> C:\path> nmake

A list of supported generators is given with:

> shell$ cmake -h

### notes

You are going to see lots of warnings during the compilation, they should only pertain to the Qt library and not the Pip3line code.

On windows you need to copy the relevant dll into the program/system folder, otherwise you end up with some missing libraries errors. There is an old-school DOS script file win_package.bat that might help you (or maybe not ..)

```
win_package.bat 2015_64
```

**Beware**: whenever developing on Windows platforms do not to mix debug and release binaries (for example the debug version of the gui using the release version of libtransform), this will lead to random memory corruptions and crashes.

## Apple OS specific

The compilation on OS X is pretty much the same as for Linux.

Just make sure to set up the CMAKE_PREFIX_PATH to the Qt base dir. For a default install it should be as follow:

> export CMAKE_PREFIX_PATH=[$HOME]/Qt/[version]/clang_64/

for example

> export CMAKE_PREFIX_PATH=/Users/user1/Qt/5.6/clang_64/

Then the rest is exactly the same as on Linux platform. 

The **OpenSSL plugin** has been disabled on this platform though.

**Python plugins** make Pip3line crash (well the Python 3 one does) when both plugins (2.7 & 3) are loaded at the same time, so you can only use one at time on OSX/MacOS. It may be because I am compiling against the stock Python2.7 and the Brew version of Python 3, but who knows, when it comes to Python everything is possible.

## OpenSSL plugin

> -DWITH_OPENSSL=ON

Defaults path for the libraries and headers have been hardcoded in the cmake configuration file on Windows  for easier use. (i.e. C:\OpenSSL-Win32 and C:\OpenSSL-Win64)

Note: the OpenSSL plugin is less relevant with QT 5, as the built-in QT 5 hashes can replace the most common hashes offered by OpenSSL.

## Python27 and Python3 plugins

> shell$ cmake ../ -DWITH_PYTHON27=ON -DWITH_PYTHON3=ON

The cmake configuration does not check for the presence of the different libraries files needed, as it  get confused when there is a dual installation  (Python 2.7 and Python 3.x).

Defaults libraries path locations are hardcoded to make things easier for the default installation environment on Windows (i.e. C:\Python27\ and C:\Python34\).

Alternatively one can specify the path by adding the following options for cmake:

Python 3 plugin
> -DPYTHON3_INCLUDE_DIRS=[PATH] -DPYTHON3_LIBRARIES=[PATH]

Python 2.7 plugin
> -DPYTHON27_INCLUDE_DIRS=[PATH] -DPYTHON27_LIBRARIES=[PATH]

For Windows download Python installation packages from:

https://www.python.org/downloads/

Be aware that you cannot mix 32bits and 64bits executables and DLLs, therefore be sure to choose the right architecture.
Unfortunately the official Python download page is not clear on what version you are downloading (32bits). If you are not sure, you are probably downloading the 32bits version, as the 64bits version is clearly indicated in the name (-amd64).

## Distorm plugin

> -DWITH_DISTORM=ON

As with the other plugins, some defaults libraries path have been hard-coded, on the basis that the Distorm source tree is at the same level as the Pip3line source directory.

Distorm can be found on GitHub and it is recommended to use the latest git version:

git clone https://github.com/gdabah/distorm.git

It is relatively easy to compile (compare to OpenSSL for instance), even on Windows platforms (hint: Have a look into the 'make' directory)
