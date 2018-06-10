/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

package burp;

import java.awt.event.ActionEvent;
import java.io.IOException;
import java.io.OutputStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import javax.swing.AbstractAction;
import javax.swing.JMenuItem;

public class Pip3lineContextMenu implements IContextMenuFactory {
	
	private ArrayList<JMenuItem> itemList;
	private IExtensionHelpers helpers;
	private IBurpExtenderCallbacks callbacks;
	private int udpSizeLimit;
	private boolean useUDP = false;
	
	private int port;
	private InetAddress host;
	public static int DEFAULT_PORT = 40000;
	public static InetAddress DEFAULT_ADDRESS = InetAddress.getLoopbackAddress();
	
	Pip3lineContextMenu(IBurpExtenderCallbacks mcallbacks) {
		callbacks = mcallbacks;
		helpers = callbacks.getHelpers();
		itemList = new ArrayList<>();
		udpSizeLimit = 65507; // UDP limit
		
		String temp = callbacks.loadExtensionSetting(BurpExtender.SEND_TO_PORT);
		try {
		port = Integer.parseInt(temp);
		} catch (NumberFormatException e){
			callbacks.issueAlert("Saved port for Pip3line is invalid, setting to 40000 (default)");
			port = DEFAULT_PORT;
			callbacks.saveExtensionSetting(BurpExtender.SEND_TO_PORT, String.valueOf(port));
		}
		
		temp = callbacks.loadExtensionSetting(BurpExtender.SEND_TO_ADDRESS);
		try {
			host = InetAddress.getByName(temp);
		} catch (UnknownHostException e){
			callbacks.issueAlert("Saved address for Pip3line is invalid, setting to localhost");
			host = DEFAULT_ADDRESS;
			callbacks.saveExtensionSetting(BurpExtender.SEND_TO_ADDRESS, host.getHostAddress());
		}
		
	}
	
	public String getAddress() {
		return host.getHostAddress();
	}
	
	public boolean setAddress(String value) {
		boolean ret = true;
		try {
			host = InetAddress.getByName(value);
			callbacks.saveExtensionSetting(BurpExtender.SEND_TO_ADDRESS, value);
		} catch (UnknownHostException e){
			callbacks.issueAlert("Invalid address for Pip3line ".concat(value));
			ret = false;
		}
		
		return ret;
	}
	
	public int getPort() {
		return port;
	}
	
	public boolean setPort(int value) {
		boolean ret = true;
		if (value != port) {
			if (value > 0 && value < 65535) {
				port = value;
				callbacks.saveExtensionSetting(BurpExtender.SEND_TO_PORT, String.valueOf(value));
			} else {
				callbacks.issueAlert("Invalid port for Pip3line");
				ret = false;
			}
		}
		
		return ret;
	}

	@Override
	public List<JMenuItem> createMenuItems(IContextMenuInvocation arg0) {
		itemList.clear();
		int tool = arg0.getToolFlag();

		if (tool == IBurpExtenderCallbacks.TOOL_INTRUDER ||
				tool == IBurpExtenderCallbacks.TOOL_PROXY ||
				tool == IBurpExtenderCallbacks.TOOL_REPEATER ||
				tool == IBurpExtenderCallbacks.TOOL_TARGET ||
				tool == IBurpExtenderCallbacks.TOOL_COMPARER) {
			
			//System.out.println(" ==> Accepted");
			JMenuItem sendToPip3line = new JMenuItem(new SendToPip3lineAction(arg0));
			itemList.add(sendToPip3line);
			return itemList;
		}
		
		return null;
	}
	
	class SendToPip3lineAction extends AbstractAction {
		private static final long serialVersionUID = -3036431674498504569L;

		private int[] selection;
		private byte[] data;
		private boolean request;
		private byte separator;
		
	    SendToPip3lineAction(IContextMenuInvocation contextMenu) {
	        super("Send to Pip3line");
	        putValue(SHORT_DESCRIPTION, "Send data to Pip3line");
	        separator = 0x0A;
	        byte contextInv = contextMenu.getInvocationContext();
	        request = contextInv == IContextMenuInvocation.CONTEXT_MESSAGE_EDITOR_REQUEST ||
	        		contextInv == IContextMenuInvocation.CONTEXT_MESSAGE_VIEWER_REQUEST ||
	        		contextInv == IContextMenuInvocation.CONTEXT_INTRUDER_PAYLOAD_POSITIONS;
	        
	        IHttpRequestResponse[] messages = contextMenu.getSelectedMessages();
	        if (messages != null && messages.length > 0) {
	        	BurpExtender.error("STP: messages.length ".concat(Integer.toString(messages.length)));
	        	if (request) {
	        		data = messages[0].getRequest();
	        	}
	        	else {
	        		data = messages[0].getResponse();
	        	}
	        	
	        	selection = contextMenu.getSelectionBounds();
	        	if (selection != null) {
		        	if (selection[0] != selection[1]) {
		        		data = Arrays.copyOfRange(data,selection[0], selection[1]);
		        	}
		        	data = helpers.base64Encode(data).getBytes();
	        	}
	        }
	    }
	    public void actionPerformed(ActionEvent e) {
	    	if (useUDP) {
	    		sendOverUDP();
	    	} else {
	    		sendOverTCP();
	    	}
	    }
	    
	    private void sendOverUDP() {
	    	if (data != null && data.length > 0) { // no point doing that if there is nothing to send
		    	DatagramSocket clientSocket;
				try {
					clientSocket = new DatagramSocket();
			        
		        	if (data.length > udpSizeLimit) { // UDP limit
		        		BurpExtender.error("SendToPip3line: packet too large for UDP, truncating");
		        		data = Arrays.copyOfRange(data,0,udpSizeLimit);
		        	}
	
			        DatagramPacket sendPacket = new DatagramPacket(data, data.length, host, port);
			        clientSocket.send(sendPacket);
				} catch (SocketException e1) {
					BurpExtender.error(e1.getMessage());
				} catch (IOException e1) {
					BurpExtender.error("Error while sending packet to pip3line".concat(e1.getMessage()));
				}
	    	}
	    }
	    
	    private void sendOverTCP() {
	    	if (data != null && data.length > 0) { // no point doing that if there is nothing to send
				try {
					Socket socket = new Socket();
					socket.connect(new InetSocketAddress(host, port), 1000);
			
					OutputStream pipeOut = socket.getOutputStream();
	
					try {
						pipeOut.write(data);
						pipeOut.write(separator);
						
					} catch (IOException e) {
						BurpExtender.error("Error while sending packet to pip3line: ".concat(e.getMessage()));
					} finally {
						socket.close();
					}
					try {
						pipeOut.close();
					} catch (IOException e) {
						BurpExtender.error("Pipe closing error: ".concat(e.getMessage()));
					}
					
				} catch (IOException e) {
					BurpExtender.error("Socket opening error: ".concat(e.getMessage()));
				}
	    	}
	    }
	}

}
