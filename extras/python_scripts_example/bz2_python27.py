#!/usr/bin/env python2
#-*- coding: UTF-8 -*-

# Released as open source by Gabriel Caudrelier
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
# https://github.com/metrodango/pip3line
# Released under AGPL see LICENSE for more information


import bz2

Pip3line_is_two_ways = True

def pip3line_transform(inputData):
	ret = None
	if (Pip3line_INBOUND):
		ret = bytearray(bz2.compress(inputData))
	else:
		ret = bytearray(bz2.decompress(inputData))
	return ret

