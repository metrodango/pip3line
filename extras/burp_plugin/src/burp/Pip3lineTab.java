/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

package burp;

import java.awt.Component;

import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel;
import javax.swing.SwingConstants;
import javax.swing.JTextField;
import javax.swing.border.TitledBorder;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import javax.swing.border.LineBorder;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import java.awt.Color;
import java.net.InetAddress;
import java.net.UnknownHostException;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.FlowLayout;

public class Pip3lineTab extends JPanel implements ITab {
	private JSpinner portSpinner;
	private static final long serialVersionUID = -2320519714434728871L;
	private JTextField hostnameTextField;
	private JTextField sendToHostnameTextField;
	private JSpinner sentToPortSpinner;
	private PayloadProcessor processor;
	private Pip3lineContextMenu contextMenu;
	private SpinnerNumberModel payloadPortModel;
	private SpinnerNumberModel sendToPortModel;

	public Pip3lineTab(PayloadProcessor mprocessor, Pip3lineContextMenu mcontextMenu) {
		processor = mprocessor;
		contextMenu = mcontextMenu;
		setLayout(new FlowLayout(FlowLayout.LEFT, 5, 5));

		JPanel mainPanel = new JPanel();
		add(mainPanel);
		mainPanel.setLayout(new BoxLayout(mainPanel, BoxLayout.Y_AXIS));
		
		JLabel lblMessage = new JLabel("All data blocks are base64 encoded");
		lblMessage.setAlignmentX(Component.LEFT_ALIGNMENT);
		mainPanel.add(lblMessage);
		
		JPanel confPanel = new JPanel();
		confPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
		mainPanel.add(confPanel);
		confPanel.setBorder(new TitledBorder(new LineBorder(new Color(184, 207, 229)),
                "Pip3line Intruder Payload processor configuration",
                TitledBorder.LEADING, TitledBorder.TOP, null, null));
		GridBagLayout gbl_confPanel = new GridBagLayout();
		gbl_confPanel.columnWidths = new int[]{220, 220, 0};
		gbl_confPanel.rowHeights = new int[]{20, 20, 0, 0, 0, 0};
		gbl_confPanel.columnWeights = new double[]{0.0, 0.0, Double.MIN_VALUE};
		gbl_confPanel.rowWeights = new double[]{0.0, 0.0, 0.0, 0.0, 0.0, Double.MIN_VALUE};
		confPanel.setLayout(gbl_confPanel);
		
		JLabel hostnameLabel = new JLabel("Hostname");
		hostnameLabel.setHorizontalAlignment(SwingConstants.LEFT);
		GridBagConstraints gbc_hostnameLabel = new GridBagConstraints();
		gbc_hostnameLabel.anchor = GridBagConstraints.WEST;
		gbc_hostnameLabel.fill = GridBagConstraints.BOTH;
		gbc_hostnameLabel.insets = new Insets(0, 0, 5, 5);
		gbc_hostnameLabel.gridx = 0;
		gbc_hostnameLabel.gridy = 0;
		confPanel.add(hostnameLabel, gbc_hostnameLabel);
		
		hostnameTextField = new JTextField();
		hostnameTextField.setHorizontalAlignment(SwingConstants.CENTER);
		hostnameTextField.setText(processor.getAddress());
		GridBagConstraints gbc_hostnameTextField = new GridBagConstraints();
		gbc_hostnameTextField.anchor = GridBagConstraints.WEST;
		gbc_hostnameTextField.insets = new Insets(0, 0, 5, 0);
		gbc_hostnameTextField.gridx = 1;
		gbc_hostnameTextField.gridy = 0;
		confPanel.add(hostnameTextField, gbc_hostnameTextField);
		hostnameTextField.setColumns(10);
		hostnameTextField.getDocument().addDocumentListener(new DocumentListener() {

			@Override
			public void insertUpdate(DocumentEvent e) {
				update();
				
			}

			@Override
			public void removeUpdate(DocumentEvent e) {
				update();
			}

			@Override
			public void changedUpdate(DocumentEvent e) {
				update();
				
			}
			
			private void update() {
				if (!processor.setAddress(hostnameTextField.getText())) {
					hostnameTextField.setBackground(Color.RED);
				} else {
					hostnameTextField.setBackground(Color.WHITE);
				}
			}
		});
		
		JLabel portLabel = new JLabel("Server Port");
		GridBagConstraints gbc_portLabel = new GridBagConstraints();
		gbc_portLabel.anchor = GridBagConstraints.WEST;
		gbc_portLabel.fill = GridBagConstraints.BOTH;
		gbc_portLabel.insets = new Insets(0, 0, 5, 5);
		gbc_portLabel.gridx = 0;
		gbc_portLabel.gridy = 1;
		confPanel.add(portLabel, gbc_portLabel);
		portLabel.setHorizontalAlignment(SwingConstants.LEFT);
		
		portSpinner = new JSpinner();
		GridBagConstraints gbc_portSpinner = new GridBagConstraints();
		gbc_portSpinner.anchor = GridBagConstraints.WEST;
		gbc_portSpinner.insets = new Insets(0, 0, 5, 0);
		gbc_portSpinner.gridx = 1;
		gbc_portSpinner.gridy = 1;
		confPanel.add(portSpinner, gbc_portSpinner);
		payloadPortModel = new SpinnerNumberModel(processor.getPort(), 1, 65535, 1);
		portSpinner.setModel(payloadPortModel);
		portSpinner.setEditor(new JSpinner.NumberEditor(portSpinner,"#"));
		
		portSpinner.addChangeListener((stateChanged -> {
				if (!processor.setPort(((Integer)portSpinner.getValue()))) {
					portSpinner.setBackground(Color.RED);
				} else {
					portSpinner.setBackground(Color.WHITE);
				}
		}));

		JButton btnPayloadResetToDefaults = new JButton("Reset to defaults");
		btnPayloadResetToDefaults.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent e) {
				hostnameTextField.setText(PayloadProcessor.DEFAULT_ADDRESS.getHostAddress());
				payloadPortModel.setValue(PayloadProcessor.DEFAULT_PORT);

			}
		});
		GridBagConstraints gbc_btnResetToDefaults = new GridBagConstraints();
		gbc_btnResetToDefaults.insets = new Insets(0, 0, 5, 0);
		gbc_btnResetToDefaults.gridx = 1;
		gbc_btnResetToDefaults.gridy = 2;
		confPanel.add(btnPayloadResetToDefaults, gbc_btnResetToDefaults);



		JPanel sendTopanel = new JPanel();
		sendTopanel.setAlignmentX(Component.LEFT_ALIGNMENT);
		sendTopanel.setBorder(new TitledBorder(new LineBorder(new Color(184, 207, 229)), "\"Send to Pip3line\" configuration", TitledBorder.LEADING, TitledBorder.TOP, null, null));
		mainPanel.add(sendTopanel);
		GridBagLayout gbl_sendTopanel = new GridBagLayout();
		gbl_sendTopanel.columnWidths = new int[] {220, 220, 0};
		gbl_sendTopanel.rowHeights = new int[] {20, 20, 0, 0, 0, 0};
		gbl_sendTopanel.columnWeights = new double[]{0.0, 0.0, Double.MIN_VALUE};
		gbl_sendTopanel.rowWeights = new double[]{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
		sendTopanel.setLayout(gbl_sendTopanel);

		JLabel lblHostname = new JLabel("Hostname");
		lblHostname.setHorizontalAlignment(SwingConstants.LEFT);
		GridBagConstraints gbc_lblHostname = new GridBagConstraints();
		gbc_lblHostname.insets = new Insets(0, 0, 5, 5);
		gbc_lblHostname.anchor = GridBagConstraints.WEST;
		gbc_lblHostname.gridx = 0;
		gbc_lblHostname.gridy = 0;
		sendTopanel.add(lblHostname, gbc_lblHostname);
		
		sendToHostnameTextField = new JTextField();
		sendToHostnameTextField.setHorizontalAlignment(SwingConstants.CENTER);
		sendToHostnameTextField.setText(contextMenu.getAddress());
		GridBagConstraints gbc_sendToHostnametextField = new GridBagConstraints();
		gbc_sendToHostnametextField.anchor = GridBagConstraints.WEST;
		gbc_sendToHostnametextField.insets = new Insets(0, 0, 5, 0);
		gbc_sendToHostnametextField.gridx = 1;
		gbc_sendToHostnametextField.gridy = 0;
		sendTopanel.add(sendToHostnameTextField, gbc_sendToHostnametextField);
		sendToHostnameTextField.setColumns(10);
		
		sendToHostnameTextField.getDocument().addDocumentListener(new DocumentListener() {

			@Override
			public void insertUpdate(DocumentEvent e) {
				update();
				
			}

			@Override
			public void removeUpdate(DocumentEvent e) {
				update();
			}

			@Override
			public void changedUpdate(DocumentEvent e) {
				update();
				
			}
			
			private void update() {
				if (!contextMenu.setAddress(sendToHostnameTextField.getText())) {
					sendToHostnameTextField.setBackground(Color.RED);
				} else {
					sendToHostnameTextField.setBackground(Color.WHITE);
				}
			}
		});

		JLabel lblPort = new JLabel("Port");
		lblPort.setHorizontalAlignment(SwingConstants.LEFT);
		GridBagConstraints gbc_lblPort = new GridBagConstraints();
		gbc_lblPort.anchor = GridBagConstraints.WEST;
		gbc_lblPort.insets = new Insets(0, 0, 5, 5);
		gbc_lblPort.gridx = 0;
		gbc_lblPort.gridy = 1;
		sendTopanel.add(lblPort, gbc_lblPort);
		
		sentToPortSpinner = new JSpinner();
		sendToPortModel = new SpinnerNumberModel(contextMenu.getPort(), 1, 65535, 1);
		sentToPortSpinner.setModel(sendToPortModel);
		JSpinner.NumberEditor editor = new JSpinner.NumberEditor(sentToPortSpinner, "#"); 
		sentToPortSpinner.setEditor(editor);
		GridBagConstraints gbc_sentToPortSpinner = new GridBagConstraints();
		gbc_sentToPortSpinner.anchor = GridBagConstraints.WEST;
		gbc_sentToPortSpinner.insets = new Insets(0, 0, 5, 0);
		gbc_sentToPortSpinner.gridx = 1;
		gbc_sentToPortSpinner.gridy = 1;
		sendTopanel.add(sentToPortSpinner, gbc_sentToPortSpinner);

		JButton btnSendToResetToDefaults = new JButton("Reset to Defaults");
		btnSendToResetToDefaults.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent e) {
				sendToHostnameTextField.setText(Pip3lineContextMenu.DEFAULT_ADDRESS.getHostAddress());
				//contextMenu.setAddress(sendToHostnameTextField.getText());
				sendToPortModel.setValue(Pip3lineContextMenu.DEFAULT_PORT);
				//contextMenu.setPort(((Integer)sentToPortSpinner.getValue()).intValue());
			}
		});
		GridBagConstraints gbc_btnResetToDefaults_1 = new GridBagConstraints();
		gbc_btnResetToDefaults_1.insets = new Insets(0, 0, 5, 0);
		gbc_btnResetToDefaults_1.gridx = 1;
		gbc_btnResetToDefaults_1.gridy = 2;
		sendTopanel.add(btnSendToResetToDefaults, gbc_btnResetToDefaults_1);

		sentToPortSpinner.addChangeListener(stateChanged -> {
            if (!contextMenu.setPort(((Integer)sentToPortSpinner.getValue()))) {
                sentToPortSpinner.setBackground(Color.RED);
            } else {
                sentToPortSpinner.setBackground(Color.WHITE);
            }
        });
		
	}

	@Override
	public String getTabCaption() {
		return "Pip3line";
	}

	@Override
	public Component getUiComponent() {
		return this;
	}
	
	public int getPort() {
		return ((Integer)portSpinner.getValue());
	}
	
	public void setPort(int val) {
		portSpinner.setValue(val);
	}
	
	public String getHostname() {
		return hostnameTextField.getText();
	}
	
	public void setHostname(String host) {
		hostnameTextField.setText(host);
	}
	
	public InetAddress getSTHostname() throws UnknownHostException {
		return InetAddress.getByName(sendToHostnameTextField.getText());
	}
	
	public void setSTHostname(InetAddress val) {
		sendToHostnameTextField.setText(val.getHostAddress());
	}
	
	public int getSTPort() {
		return ((Integer)sentToPortSpinner.getValue());
	}
	
	public void setSTPort(int val) {
		sentToPortSpinner.setValue(val);
	}
}
