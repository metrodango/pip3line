/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

package burp;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;

public class PayloadProcessor implements IIntruderPayloadProcessor {
	
	private IBurpExtenderCallbacks mcallbacks;
	private IExtensionHelpers helpers;
	private Pip3lineTab tab;
	private byte separator;
	private int port;
	private InetAddress host;
	private boolean urlEncodeOutput;
	private Socket socket;
	private OutputStream pipeOut;
	private InputStream pipeIn;
	
	public static int DEFAULT_PORT = 45632;
	public static InetAddress DEFAULT_ADDRESS = InetAddress.getLoopbackAddress();
	
	public PayloadProcessor(Pip3lineTab configTab, IBurpExtenderCallbacks callbacks) {
		mcallbacks = callbacks;
		helpers = mcallbacks.getHelpers();
		tab = configTab;
		
		String temp = mcallbacks.loadExtensionSetting(BurpExtender.PROCESSOR_PORT);
		try {
		port = Integer.parseInt(temp);
		} catch (NumberFormatException e){
			mcallbacks.issueAlert("Saved port for Payload processor is invalid, setting to 45632 (default)");
			port = DEFAULT_PORT;
			mcallbacks.saveExtensionSetting(BurpExtender.PROCESSOR_PORT, String.valueOf(port));
		}
		
		temp = mcallbacks.loadExtensionSetting(BurpExtender.PROCESSOR_ADDRESS);
		try {
			host = InetAddress.getByName(temp);
		} catch (UnknownHostException e){
			mcallbacks.issueAlert("Saved address for Payload processor is invalid, setting to localhost");
			host = DEFAULT_ADDRESS;
			mcallbacks.saveExtensionSetting(BurpExtender.PROCESSOR_ADDRESS, host.getHostAddress());
		}
		
		separator = 0x0A;
		socket = null;
		pipeOut = null;
		pipeIn = null;
		urlEncodeOutput = true;
	}
	
	public int getPort() {
		return port;
	}
	
	public boolean setPort(int value) {
		boolean ret = true;
		if (value != port) {
			if (value > 0 && value < 65535) {
				port = value;
				mcallbacks.saveExtensionSetting(BurpExtender.PROCESSOR_PORT, String.valueOf(value));
			} else {
				mcallbacks.issueAlert("Invalid port for Payload processor");
				ret = false;
			}
		}
		
		return ret;
	}
	
	public String getAddress() {
		return host.getHostAddress();
	}
	
	public boolean setAddress(String value) {
		boolean ret = true;
		try {
			host = InetAddress.getByName(value);
			mcallbacks.saveExtensionSetting(BurpExtender.PROCESSOR_ADDRESS, value);
		} catch (UnknownHostException e){
			mcallbacks.issueAlert("Invalid address for Payload processor");
			ret = false;
		}
		
		return ret;
	}

	@Override
	public String getProcessorName() {
		return "Pip3line";
	}

	@Override
	public byte[] processPayload(byte[] currentPayload, byte[] originalPayload,
			byte[] baseValue) {
		return internalProcessing(currentPayload);
	}
	
	private byte[] internalProcessing(byte[] currentPayload) {
		tab.setEnabled(false);

		urlEncodeOutput = true;

		byte[] returnValue = null;
		byte[] separators = { separator };

		try {
			if (socket == null || socket.isClosed()) {
				socket = new Socket(host, port);
				pipeOut = socket.getOutputStream();
				pipeIn = socket.getInputStream();
			}
            returnValue = null;
            byte[] data = helpers.urlDecode(currentPayload);

            data = helpers.stringToBytes(helpers.base64Encode(data));
            
            
			try {
				pipeOut.write(data);
				pipeOut.write(separators);
				
		//		System.out.println("Data block send: ".concat(helpers.bytesToString(data)));
				
				int byteRead = pipeIn.read();
				ArrayList<Byte> returnedVals = new ArrayList<Byte>();
				while (byteRead != -1 && byteRead != separator) {
					returnedVals.add((byte) byteRead);
					byteRead = pipeIn.read();
				}
			
				returnValue = new byte[returnedVals.size()];
				
				for (int i = 0; i < returnedVals.size(); i++) {
					returnValue[i] = returnedVals.get(i).byteValue();
				}
				
			//	System.out.println("Data block read: ".concat(helpers.bytesToString(returnValue)));
				returnValue  = helpers.base64Decode(returnValue);
				
				if (urlEncodeOutput) {
					returnValue  = helpers.urlEncode(returnValue);
				}
				
				socket.close();
				
			} catch (IOException e) {
				BurpExtender.error("Pipe error: ".concat(e.getMessage()));
			}
			try {
				pipeOut.close();
				pipeIn.close();
			} catch (IOException e) {
				BurpExtender.error("Pipe closing error: ".concat(e.getMessage()));
			}
			
		} catch (IOException e) {
			BurpExtender.error("Socket opening error: ".concat(e.getMessage()));
		}
		
		tab.setEnabled(true);
		return returnValue;
		
	}

}
