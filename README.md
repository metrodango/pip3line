# Pip3line

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/570b18ced7044b7193d49fc1c9409316)](https://www.codacy.com/app/metrodango/pip3line?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=metrodango/pip3line&amp;utm_campaign=Badge_Grade)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/metrodango/pip3line.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/metrodango/pip3line/alerts/)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/metrodango/pip3line.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/metrodango/pip3line/context:cpp)
[![Language grade: Python](https://img.shields.io/lgtm/grade/python/g/metrodango/pip3line.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/metrodango/pip3line/context:python)

Linux / MacOS Builds

[![Build Status](https://app.travis-ci.com/metrodango/pip3line.svg?branch=master)](https://travis-ci.com/metrodango/pip3line)

Windows Builds

[![Build status](https://ci.appveyor.com/api/projects/status/5ck7rc2sq4py9u39/branch/master?svg=true)](https://ci.appveyor.com/project/metrodango/pip3line/branch/master)

Raw bytes manipulation utility, able to apply well known and less well known transformations, quick binary search, network proxy interception and more.

## Install on Windows:

Download the release zip [here](https://github.com/metrodango/pip3line/releases)

**Remember to install/repair the VS redistributables from the zip file**

## Install on MacOs:

~~~
$ brew tap metrodango/pip3line
$ brew install pip3line
~~~

> If an error occur at start regarding missing shared library libtransform, make sure that the homebrew *lib* directory is in the path, or just create a link it from */usr/local/lib*

## Install on Linux:

### Gentoo

copy the template ebuild from *extras/gentoo_package/pip3line-TEMPLATE.ebuild*  into a local Gentoo repository, with the relevant versioning info, for example:

~~~
pip3line-9999.ebuild (for the github based ebuild)
pip3line-3.7.0.ebuild (for a specific version)
~~~

then

~~~
$ emerge -pv pip3line
~~~

### Other distributions

For now see the [build page](https://github.com/metrodango/pip3line/blob/master/Build-process.md)

## Documentation

[Main site]

http://metrodango.github.io/pip3line/index.html

[Python plugins]

https://github.com/metrodango/pip3line/wiki/Python-2.7-&-3-plugins

[Native plugins]

https://github.com/metrodango/pip3line/wiki/Coding-your-own-plugins

[Build process]

https://github.com/metrodango/pip3line/blob/master/Build-process.md

~~~
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL-3.0 see LICENSE for more information
~~~


