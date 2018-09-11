#include "ConnectionMenuFrame.h"
#include "ui_ConnectionMenuFrame.h"

ConnectionMenuFrame::ConnectionMenuFrame(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::ConnectionMenuFrame)
{
	ui->setupUi(this);

	setUIState(UIState_Initializing);

	m_bAutoPairing = false;

	connect(ui->keyboardFrame, SIGNAL(signalAccepted()), this, SLOT(slotEnterFromKeyboard()));
	connect(ui->keyboardFrame, SIGNAL(signalRejected()), this, SLOT(slotExitFromKeyboard()));

	ui->keyboardFrame->hide();

	ui->BUTTON_RENAME_BT_DEVICE->installEventFilter(this);
	ui->BUTTON_UNPAIR->installEventFilter(this);
	ui->BUTTON_UNPAIR_ALL->installEventFilter(this);
	ui->LISTWIDGET_PAIRED_DEVICE->viewport()->installEventFilter(this);
}

ConnectionMenuFrame::~ConnectionMenuFrame()
{
	delete ui;
}

bool ConnectionMenuFrame::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == ui->BUTTON_RENAME_BT_DEVICE || watched == ui->BUTTON_UNPAIR || watched == ui->BUTTON_UNPAIR_ALL || watched == ui->LISTWIDGET_PAIRED_DEVICE->viewport())
	{
		if (event->type() == QEvent::MouseButtonPress && !ui->keyboardFrame->isHidden())
		{
			ui->keyboardFrame->hide();
		}
	}

	return QFrame::eventFilter(watched, event);
}

void ConnectionMenuFrame::mousePressEvent(QMouseEvent *e)
{
	if (e->x() < ui->keyboardFrame->x() || ui->keyboardFrame->x() <= e->x() || e->y() < ui->keyboardFrame->y() || ui->keyboardFrame->y() <= e->y())
	{
		ui->keyboardFrame->hide();
	}
}

void ConnectionMenuFrame::on_BUTTON_RENAME_BT_DEVICE_clicked()
{
	ui->keyboardFrame->move(52, 38);
	ui->keyboardFrame->setText(ui->LABEL_BT_DEVICE_NAME->text());
	ui->keyboardFrame->show();
	ui->keyboardFrame->raise();
}

void ConnectionMenuFrame::slotEnterFromKeyboard()
{
	QString name = ui->keyboardFrame->text();;
	if (name.isEmpty())
		return;

	// If the current name and the name you want to change are the same, ignore it.
	if (ui->LABEL_BT_DEVICE_NAME->text() == name)
		return;

	emit signalCommandToServer("$MGT#RENAME LOCAL DEVICE " + name + "\n");

	ui->keyboardFrame->hide();
}

void ConnectionMenuFrame::slotExitFromKeyboard()
{
	ui->keyboardFrame->hide();
}

void ConnectionMenuFrame::on_BUTTON_UNPAIR_clicked()
{
	int row = ui->LISTWIDGET_PAIRED_DEVICE->currentRow();
	if (row < 0)
		return;

	emit signalCommandToServer(QString("$MGT#UNPAIR DEVICE %1\n").arg(row));
}

void ConnectionMenuFrame::on_BUTTON_UNPAIR_ALL_clicked()
{
	if (ui->LISTWIDGET_PAIRED_DEVICE->count() == 0)
		return;

	emit signalCommandToServer("$MGT#UNPAIR DEVICE ALL\n");
}

void ConnectionMenuFrame::setUIState(UIState state)
{
	m_UIState = state;
}

void ConnectionMenuFrame::slotCommandFromServer(QString command)
{
	int stx = command.indexOf("$");
	int etx = command.indexOf("\n");

	// assume unknown command format.
	if (stx < 0 || etx < 0) {
		return;
	}

	// body = remove STX and ETX from command
	stx++;
	QString body = command.mid(stx, etx-stx);

	// example) command = "$OK#MGT#LOCAL DEVICE NAME\n"
	// example) body    = "OK#MGT#LOCAL DEVICE NAME"

	QStringList tokens = body.split("#");

	// failure conditions
	if (tokens.size() < 3) {
		return;
	} else if (tokens[0] == "NG") {
		return;
	}

	// valid commands
	if (tokens[2] == "PING" && m_UIState == UIState_Initializing) {
		// 1. initialize
		// 1.1. current system BT device name
		emit signalCommandToServer("$MGT#LOCAL DEVICE NAME\n");
		// 1.2. current system BT device MAC address
		emit signalCommandToServer("$MGT#LOCAL DEVICE ADDRESS\n");
		// 1.3. paired devices infomation
		emit signalCommandToServer("$MGT#PAIRED DEVICE INFO ALL LIST\n");

		setUIState(UIState_Initialized);
	} else if (tokens[2] == "LOCAL DEVICE NAME") {
		updateToUIForLocalDeviceName(tokens);
	} else if (tokens[2].indexOf("RENAME LOCAL DEVICE") == 0) {
		emit signalCommandToServer("$MGT#LOCAL DEVICE NAME\n");
	} else if (tokens[2] == "LOCAL DEVICE ADDRESS") {
		updateToUIForLocalDeviceAddress(tokens);
	} else if (tokens[2] == "PAIRED DEVICE INFO ALL LIST") {
		updateToUIForPairedDeviceList(tokens);
	} else if (tokens[2] == "PAIRING REQUEST") {
//		updateToUIForPairingRequest(tokens);
	} else if (tokens[2] == "CONNECTION STATUS") {
		updateToUIForConnectionStatus(tokens);
	} else if (tokens[2] == "CONNECTED DEVICE INDEX") {
		updateToUIForConnectedDeviceIndex(tokens);
	} else if (tokens[2] == "PAIRING FAILED") {
//		updateToUIForPairFailed(tokens);
	} else if (tokens[2] == "AUTO PAIRING") {
		updateForAutoPairing(tokens);
	}
}

void ConnectionMenuFrame::updateToUIForLocalDeviceName(QStringList& tokens)
{
	if (tokens.size() == 4) {
		ui->LABEL_BT_DEVICE_NAME->setText(tokens[3]);
	}
}

void ConnectionMenuFrame::updateToUIForLocalDeviceAddress(QStringList& tokens)
{
	if (tokens.size() == 4) {
		ui->LABEL_BT_DEVICE_MAC_ADDRESS->setText(tokens[3]);
	}
}

void ConnectionMenuFrame::updateToUIForPairedDeviceList(QStringList& tokens)
{
	// example) $OK#MGT#PAIRED DEVICE INFO ALL LIST
	QListWidgetItem* item = NULL;
	BTPairedDeviceItem* custom = NULL;
	// array (position, seperator) description
	// --------------------------------------------
	// index  |      description
	//   0    | stx position/character
	//   1    | seperator position/character
	//   2    | etx position/character
	// --------------------------------------------
	int position_array[3];
	QChar seperator_array[3] = {'<', ',', '>'};
	QString roi;
	QStringList subTokens;

	ui->LISTWIDGET_PAIRED_DEVICE->clear();

	if (tokens.size() > 3) {
		for (int i = 3; i < tokens.size(); i++) {
			for (int k = 0; k < 3; k++) {
				position_array[k] = tokens[i].indexOf(seperator_array[k]);
				if (position_array[k] < 0) // if current text is invalid format, skip.
					goto next_loop;
			}

		   item = new QListWidgetItem;
		   item->setSizeHint(QSize(ui->LISTWIDGET_PAIRED_DEVICE->width()-2, 100));
		   custom = new BTPairedDeviceItem(this);
		   custom->setFixedSize(ui->LISTWIDGET_PAIRED_DEVICE->width()-2, 100); //  -2 is left/right border size (left:1 , right:1)
		   connect(custom, SIGNAL(signalAVKConnectButtonClicked(int,BTPairedDeviceItem*)), this, SLOT(slotAVKConnectButtonClicked(int,BTPairedDeviceItem*)));
		   connect(custom, SIGNAL(signalHSConnectButtonClicked(int,BTPairedDeviceItem*)), this, SLOT(slotHSConnectButtonClicked(int,BTPairedDeviceItem*)));

		   // remove '<' and '>'
		   roi = tokens[i].mid(1, tokens[i].length()-2);
		   subTokens = roi.split(",");

		   if (subTokens.size() != 4)
			   continue;

		   // subtokens index description
		   //             0   BT device name
		   //             1   BT device address
		   //             2   BT device avk connection state
		   //			  3	  BT device hs connection state

		   custom->setDeviceName(subTokens[0]);
		   custom->setDeviceAddress(subTokens[1]);
		   custom->setConnectionStatusForAVK(subTokens[2]);
		   custom->setConnectionStatusForHS(subTokens[3]);
		   custom->setRow(ui->LISTWIDGET_PAIRED_DEVICE->count());

		   ui->LISTWIDGET_PAIRED_DEVICE->addItem(item);
		   ui->LISTWIDGET_PAIRED_DEVICE->setItemWidget(item, custom);
next_loop:
			continue;
		}
	}

//	if (m_bAutoPairing && !m_pPairingRequestDialog->isHidden())
//		m_pPairingRequestDialog->hide();

	// if list widget item count is positive, enable unpair buttons.
	// else disable.
	ui->BUTTON_UNPAIR->setEnabled(ui->LISTWIDGET_PAIRED_DEVICE->count());
	ui->BUTTON_UNPAIR_ALL->setEnabled(ui->LISTWIDGET_PAIRED_DEVICE->count());
}

//void ConnectionMenuFrame::updateToUIForPairingRequest(QStringList& tokens)
//{
//	// example) $OK#MGT#PAIRING REQUEST#AUTO ON#iPhone6#28:5a:eb:78:2a:63#095999\n

//	if (tokens.size() == 7) {
//		if (tokens[3] == "AUTO ON") {
//			m_pPairingRequestDialog->setButtonStatePolicy(MessageDialog::ButtonStatePolicy_Only_Ok);
//			m_pPairingRequestDialog->setButtonText(MessageDialog::ButtonType_Ok, "OK");
//		} else if (tokens[3] == "AUTO OFF") {
//			m_pPairingRequestDialog->setButtonStatePolicy(MessageDialog::ButtonStatePolicy_Ok_And_Cancel);
//			m_pPairingRequestDialog->setButtonText(MessageDialog::ButtonType_Ok, "Pairing");
//		} else {
//			return;
//		}

//		// if auto connection is OFF, show dialog for paring request.
//		m_pPairingRequestDialog->setTitle("<b>Bluetooth pairing request<b/>");

//		m_pPairingRequestDialog->setMessage(QString("<p align=\"left\">Please check your authorization number to connect with the '%1' device.<br>Device address : %2<br><p align=\"center\"><font size=\"12\" color=\"blue\">%3</font></p>").arg(tokens[4]).arg(tokens[5]).arg(tokens[6]));

//		m_pPairingRequestDialog->exec();
//	}
//}

//void ConnectionMenuFrame::slotPairingRequestAccepted()
//{
//	emit signalCommandToServer("$MGT#ACCEPT PAIRING\n");
//}

//void ConnectionMenuFrame::slotPairingRequestRejected()
//{
//	emit signalCommandToServer("$MGT#REJECT PAIRING\n");
//}

void ConnectionMenuFrame::updateToUIForConnectionStatus(QStringList& tokens)
{
	// example) "$OK#HS#CONNECTION STATUS#CONNECTED#Nexus 7#50:46:5d:13:3c:a0\n"

	// assume invalid reply command.
	if (tokens.size() < 6)
		return;

	// if connection status is connected, run request command for connected device index.
	if (tokens[3] == "CONNECTED") {
		if (tokens[1] == "AVK") {
			emit signalCommandToServer("$AVK#CONNECTED DEVICE INDEX\n");
		} else if (tokens[1] == "HS") {
			emit signalCommandToServer("$HS#CONNECTED DEVICE INDEX\n");
		}

		return;
	}

	// if connection status is disconnected
	if (tokens[1] == "AVK") {
		for (int i = 0; i < ui->LISTWIDGET_PAIRED_DEVICE->count(); i++) {
			QListWidgetItem* item = ui->LISTWIDGET_PAIRED_DEVICE->item(i);
			BTPairedDeviceItem* custom = (BTPairedDeviceItem*)ui->LISTWIDGET_PAIRED_DEVICE->itemWidget(item);
			custom->setConnectionStatusForAVK("DISCONNECTED");
		}
	} else if (tokens[1] == "HS") {
		for (int i = 0; i < ui->LISTWIDGET_PAIRED_DEVICE->count(); i++) {
			QListWidgetItem* item = ui->LISTWIDGET_PAIRED_DEVICE->item(i);
			BTPairedDeviceItem* custom = (BTPairedDeviceItem*)ui->LISTWIDGET_PAIRED_DEVICE->itemWidget(item);
			custom->setConnectionStatusForHS("DISCONNECTED");
		}
	}
}

void ConnectionMenuFrame::updateToUIForConnectedDeviceIndex(QStringList& tokens)
{
	// example) "$OK#AVK#CONNECTED DEVICE INDEX#1\n"

	if (tokens.size() == 4) {
		int index = tokens[3].toInt();

		if (tokens[1] == "AVK") {
			for (int i = 0; i < ui->LISTWIDGET_PAIRED_DEVICE->count(); i++) {
				QListWidgetItem* item = ui->LISTWIDGET_PAIRED_DEVICE->item(i);
				if (!item)
					continue;

				BTPairedDeviceItem* custom = (BTPairedDeviceItem*)ui->LISTWIDGET_PAIRED_DEVICE->itemWidget(item);
				custom->setConnectionStatusForAVK(index == i ? "CONNECTED" : "DISCONNECTED");
			}
		} else if (tokens[1] == "HS") {
			for (int i = 0; i < ui->LISTWIDGET_PAIRED_DEVICE->count(); i++) {
				QListWidgetItem* item = ui->LISTWIDGET_PAIRED_DEVICE->item(i);
				if (!item)
					continue;

				BTPairedDeviceItem* custom = (BTPairedDeviceItem*)ui->LISTWIDGET_PAIRED_DEVICE->itemWidget(item);
				custom->setConnectionStatusForHS(index == i ? "CONNECTED" : "DISCONNECTED");
			}
		}
	}
}

void ConnectionMenuFrame::slotAVKConnectButtonClicked(int row, BTPairedDeviceItem* custom)
{
	if (row < 0)
		return;

	QListWidgetItem* item = ui->LISTWIDGET_PAIRED_DEVICE->item(row);
	BTPairedDeviceItem* verify = (BTPairedDeviceItem*)ui->LISTWIDGET_PAIRED_DEVICE->itemWidget(item);

	if (!item || !custom) {
		return;
	} else if (verify != custom) {
		return;
	}

	if (custom->connectionStatusForAVK() == "CONNECTED") {
		emit signalCommandToServer(QString("$AVK#DISCONNECT %1\n").arg(row));
	} else if (custom->connectionStatusForAVK() == "DISCONNECTED") {
		emit signalCommandToServer(QString("$AVK#CONNECT %1\n").arg(row));
	}
}

void ConnectionMenuFrame::slotHSConnectButtonClicked(int row, BTPairedDeviceItem* custom)
{
	if (row < 0)
		return;

	QListWidgetItem* item = ui->LISTWIDGET_PAIRED_DEVICE->item(row);
	BTPairedDeviceItem* verify = (BTPairedDeviceItem*)ui->LISTWIDGET_PAIRED_DEVICE->itemWidget(item);

	if (!item || !custom) {
		return;
	} else if (verify != custom) {
		return;
	}

	if (custom->connectionStatusForHS() == "CONNECTED") {
		emit signalCommandToServer(QString("$HS#DISCONNECT %1\n").arg(row));
	} else if (custom->connectionStatusForHS() == "DISCONNECTED") {
		emit signalCommandToServer(QString("$HS#CONNECT %1\n").arg(row));
	}
}

//void ConnectionMenuFrame::updateToUIForPairFailed(QStringList& tokens)
//{
//	if (tokens.size() == 4) {
//		if (tokens[3] == "5") {
//			if (m_bAutoPairing) {
//				if (!m_pPairingRequestDialog->isHidden())
//					m_pPairingRequestDialog->hide();
//			}
//		}
//	}
//}

void ConnectionMenuFrame::updateForAutoPairing(QStringList& tokens)
{
	if (tokens.size() == 4) {
		m_bAutoPairing = (tokens[3] == "ON");
	}
}
