#!/usr/bin/env python3
#-*- coding: UTF-8 -*-

# Released as open source by Gabriel Caudrelier
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
# https://github.com/metrodango/pip3line
# Released under AGPL see LICENSE for more information

import gzip

Pip3line_is_two_ways = True

def pip3line_transform(inputData):
	ret = None
	if (Pip3line_INBOUND):
		ret = bytearray(gzip.compress(inputData))
	else:
		ret = bytearray(gzip.decompress(inputData))
	return ret

