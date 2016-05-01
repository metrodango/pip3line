#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Released as open source by Gabriel Caudrelier
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
# https://github.com/metrodango/pip3line
# Released under AGPL see LICENSE for more information

# This script is just an example on how to communicate with the pip3line gui via sockets

import re
import sys
import argparse
import socket
import base64

class Pip3line:
	DEFAULT_TCP = ('127.0.0.1',45632)
	DEFAULT_UNIX_SOCKET="/tmp/pip3lineMass"
	DEFAULT_WINDOWS_PIPE="pip3lineMass"

	def __init__(self):
		self.DEFAULT_TCP = ('127.0.0.1',45632)
		self.DEFAULT_UNIX_SOCKET="/tmp/pip3lineMass"
		self.DEFAULT_WINDOWS_PIPE="pip3lineMass"
		self.tcpparam = self.DEFAULT_TCP
		self.base64inUse = False
		self.conn = None
	
	def __del__(self):
		if self.conn != None:
			self.conn.close()

	def setup(self,tcp=None, pipe=None):
		if (tcp != None):
			self.tcpparam = tcp
			if self.connect():
				print("TCP connection opened on",str(tcp))
		elif (pipe != None):
			if os.path.exists(pipe):
			        self.conn = open(pipe, 'r')
			else:
				print("no Unix Socket/Named Pipe for  " + pipe)

			print("Pipe connection opened on",pipe)
	def connect(self):
		try:
			self.conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		except socket.error as msg:
			self.networkError(msg)
			self.conn = None
			return False

		try:
			self.conn.connect(self.tcpparam)
		except socket.error as msg:
			self.networkError(msg)
			self.conn.close()
			self.conn = None
			return False

		return True

	def __sending(self,data):
		if self.base64inUse:
			data = base64.b64encode(data)
		
		self.conn.send(data)
		self.conn.send(b'\n')
		
		endBlock = False
		rdata = b''
		while not endBlock:
			rdata += self.conn.recv(1024)
			if rdata.count('\n') > 0:
				rdata = rdata.split('\n')[0]
				endBlock = True


		if rdata != b'' and self.base64inUse:
			rdata = base64.b64decode(rdata)

		return rdata

	
	def process(self,data):
		if type(data) != type(b''):
			print("Need array of bytes")
			return b''

		if (self.conn == None):
			if not self.connect():
				return b''

		try:
			rdata = self.__sending(data)
			if rdata==b'':
				print("Connection died (retrying)")
				if self.connect():
					rdata = self.__sending(data)
					if rdata==b'':
						self.networkError("Server down?")
		except socket.error as msg:
			self.networkError(msg)
			if self.connect():
				rdata = self.__sending(data)

		return rdata
	def networkError(self,msg):
		print(msg,str(self.tcpparam))

if __name__ == "__main__":
	print("Running tests")
