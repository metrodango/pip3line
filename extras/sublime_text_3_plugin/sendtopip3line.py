
import sublime, sublime_plugin
import base64
import socket

class SendToPip3line(sublime_plugin.TextCommand):
	def __init__(self,args):
		sublime_plugin.TextCommand.__init__(self,args)
		self.settings = sublime.load_settings("pip3line.sublime-settings")
		self.adress = self.settings.get("ipaddress", "127.0.0.1")
		self.port = self.settings.get("port",40000)
		self.separator = bytes.fromhex(self.settings.get("separator","0a"))

	def is_enabled(self):
		return (len(self.view.sel()) > 0 and not self.view.sel()[0].empty())

	def run(self, edit):
		try:
			for region in self.view.sel():
				if not region.empty():
					s = self.view.substr(region)
					s = base64.b64encode(s.encode("utf-8"))
					self.sendData(s)
		except e:
			print("SendToPip3line Error:" + str(e))
	def sendData(self,data):
		try:
			sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			sock.settimeout(3)
			sock.connect((self.adress, self.port))
			sock.settimeout(None)
			totalsent = 0
			data += b'\x0a'
			#print(str(data))
			while totalsent < len(data):
				sent = sock.send(data[totalsent:])
				if sent == 0:
					raise RuntimeError("socket connection broken")
				totalsent = totalsent + sent
			sock.close()
		except ConnectionRefusedError:
			print("SendToPip3line Network connection refused. Is Pip3line running?")
		except IOError as e:
			print("SendToPip3line Network error [" + self.adress + ":"+ str(self.port) + "]: " + str(e))
		except e:
			print("SendToPip3line Error:" + str(e))
