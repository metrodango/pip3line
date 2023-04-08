#!/usr/bin/env python
#-*- coding: UTF-8 -*-

# Released as open source by Gabriel Caudrelier
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
# https://github.com/metrodango/pip3line
# Released under BSD 3-Clause see LICENSE for more information

# [Optional] you can precise that this transformation is bidirectional and check for the
# Pip3line_INBOUND module attribute later on (see gzip and bz2 for examples)
Pip3line_is_two_ways = False

# [Optional] one can also define default parameter names like this
# (they have to be unicode strings, if not they will be ignored)
Pip3line_params_names = ["param1", "param2"]

# You need to implement a function with this exact name

def pip3line_transform(inputData):
	""" Take a bytearray as input and needs to return a bytearray"""
	# if you need to convert to a python 'string'
	string = inputData.decode('utf-8')
	string = string[::-1]

	# if there was paramters defined in the gui, here they are as a unicode string dict
	print(str(Pip3line_params))

	# just remember to convert it back to a bytearray
	return bytearray(string, 'utf-8')
