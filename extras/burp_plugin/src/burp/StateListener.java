package burp;

public class StateListener implements IExtensionStateListener {
	private BurpExtender extender;
	
	public StateListener(BurpExtender mextender) {
		extender = mextender;
		
	}

	public void extensionUnloaded() {
		extender.unregister();
	}
}
