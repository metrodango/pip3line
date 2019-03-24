# Pip3line

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/570b18ced7044b7193d49fc1c9409316)](https://www.codacy.com/app/metrodango/pip3line?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=metrodango/pip3line&amp;utm_campaign=Badge_Grade)

Linux / MacOS Builds

[![Build Status](https://travis-ci.org/metrodango/pip3line.svg?branch=master)](https://travis-ci.org/metrodango/pip3line)

Windows Builds

[![Build status](https://ci.appveyor.com/api/projects/status/5ck7rc2sq4py9u39/branch/master?svg=true)](https://ci.appveyor.com/project/metrodango/pip3line/branch/master)

Raw bytes manipulation utility, able to apply well known and less well known transformations.

## Install on Windows:

Download the release zip [here](https://github.com/metrodango/pip3line/releases)

**Remember to install/repair the VS redistributables from the zip file**

## Install on MacOs:

~~~
$ brew install metrodango/pip3line/pip3line
~~~

## Install on Linux:

### Gentoo

If using the 9999 package:

~~~
$ emerge -pv pip3line
~~~

If using the normal package, disable the network-sandbox for distorm

~~~
$ FEATURES="-network-sandbox" emerge pip3line
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


