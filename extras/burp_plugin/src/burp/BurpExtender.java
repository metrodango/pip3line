/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

package burp;

import java.io.PrintWriter;

public class BurpExtender implements IBurpExtender {
	private Pip3lineTab tab;
	private static IBurpExtenderCallbacks burp = null;
	private PayloadProcessor processor;
	private static PrintWriter errOut = null;
	private static PrintWriter stdOut = null;
	private Pip3lineContextMenu contextMenuFactory;
	private StateListener stateListener;
	
	public static String PROCESSOR_PORT = "ProcessorPort";
	public static String PROCESSOR_ADDRESS = "ProcessorAddress";
	public static String SEND_TO_PORT = "SendToPort";
	public static String SEND_TO_ADDRESS = "SendToAddress";
	
	public BurpExtender() {

		tab = null;
		contextMenuFactory = null;
		processor = null;
	}


	@Override
	public void registerExtenderCallbacks(IBurpExtenderCallbacks callbacks) {
		
		errOut = new PrintWriter(callbacks.getStderr());
		stdOut = new PrintWriter(callbacks.getStdout());
		processor = new PayloadProcessor(tab, callbacks);
		contextMenuFactory = new Pip3lineContextMenu(callbacks);
		stateListener = new StateListener(this);
		tab = new Pip3lineTab(processor,contextMenuFactory);
		callbacks.registerIntruderPayloadProcessor(processor);
		callbacks.registerContextMenuFactory(contextMenuFactory);
		callbacks.setExtensionName("Pip3line plugin");
		callbacks.addSuiteTab(tab);
		callbacks.registerExtensionStateListener(stateListener);
		burp = callbacks;
		alert("Initialized");
	}

	public void unregister() {
		//burp.issueAlert("Unloaded");
	}
	
	public IBurpExtenderCallbacks getBurpCallBacks() {
		return burp;
	}
	
	public static final void alert(String message) {
		burp.issueAlert(message);
		stdOut.println(message);
	}
	
	public static final void error(String message) {
		burp.issueAlert(message);
		errOut.println(message);
	}
	
	public static final void message (String message) {
		stdOut.println(message);
	}
	

}
