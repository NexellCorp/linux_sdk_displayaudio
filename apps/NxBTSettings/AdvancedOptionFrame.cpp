#include "AdvancedOptionFrame.h"
#include "ui_AdvancedOptionFrame.h"
#include <QDesktopWidget>

#define DEFAULT_WIDTH	1024
#define DEFAULT_HEIGHT	540

AdvancedOptionFrame::AdvancedOptionFrame(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::AdvancedOptionFrame)
{
	ui->setupUi(this);

	setUIState(UIState_Initializing);

	const QRect screen = QApplication::desktop()->screenGeometry();
	if ((width() != screen.width()) || (height() != screen.height()))
	{
		setFixedSize(screen.width(), screen.height() * 0.9);
	}
}

AdvancedOptionFrame::~AdvancedOptionFrame()
{
	delete ui;
}

void AdvancedOptionFrame::setUIState(UIState state)
{
	m_UIState = state;
}

void AdvancedOptionFrame::slotCommandFromServer(QString command)
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
	} else if (tokens[1] != "MGT") {
		return;
	}

	// valid commands
	if (tokens[2] == "PING" && m_UIState == UIState_Initializing) {
		QStringList commands;
		// 1. initialize
		// 1.1. auto connection state
		emit signalCommandToServer("$MGT#AUTO CONNECTION\n");
		// 1.2. auto pairing state
		emit signalCommandToServer("$MGT#AUTO PAIRING\n");

		setUIState(UIState_Initialized);
	} else if (tokens[2] == "AUTO CONNECTION") {
		updateToUIForAutoConnection(tokens);
	} else if (tokens[2].startsWith("ENABLE AUTO CONNECTION")) {
		updateToUIForAutoConnection(tokens);
		sync();
	} else if (tokens[2] == "AUTO PAIRING") {
		updateToUIForAutoPairing(tokens);
	} else if (tokens[2].startsWith("ENABLE AUTO PAIRING")) {
		updateToUIForAutoPairing(tokens);
		sync();
	}
}

void AdvancedOptionFrame::setUIStateForAutoSettings(AutoSettingsList type, AutoSettingsState state)
{
	switch (type) {
	case AutoSettingsList_AutoConnection:
		m_AutoConnectionState = state;

		switch (state) {
		case AutoSettingsState_ON:
			ui->BUTTON_AUTO_CONNECTION_ON->show();
			ui->BUTTON_AUTO_CONNECTION_OFF->hide();
			break;

		case AutoSettingsState_OFF:
			ui->BUTTON_AUTO_CONNECTION_OFF->show();
			ui->BUTTON_AUTO_CONNECTION_ON->hide();
			break;
		}
		break;

	case AutoSettingsList_AutoPairing:
		m_AutoPairingState = state;

		switch (state) {
		case AutoSettingsState_ON:
			ui->BUTTON_AUTO_PAIRING_ON->show();
			ui->BUTTON_AUTO_PAIRING_OFF->hide();
			break;

		case AutoSettingsState_OFF:
			ui->BUTTON_AUTO_PAIRING_OFF->show();
			ui->BUTTON_AUTO_PAIRING_ON->hide();
			break;
		}
		break;
	}
}

void AdvancedOptionFrame::updateToUIForAutoConnection(QStringList& tokens)
{
	if (tokens.size() == 4 && tokens[2].startsWith("AUTO")) {
		if (tokens[3] == "ON")
			setUIStateForAutoSettings(AutoSettingsList_AutoConnection, AutoSettingsState_ON);
		else if (tokens[3] == "OFF")
			setUIStateForAutoSettings(AutoSettingsList_AutoConnection, AutoSettingsState_OFF);

	} else if (tokens.size() == 3 && tokens[2].startsWith("ENABLE")) {
		if (tokens[2].right(2) == "ON")
			setUIStateForAutoSettings(AutoSettingsList_AutoConnection, AutoSettingsState_ON);
		else if (tokens[2].right(3) == "OFF")
			setUIStateForAutoSettings(AutoSettingsList_AutoConnection, AutoSettingsState_OFF);
	}
}

void AdvancedOptionFrame::updateToUIForAutoPairing(QStringList& tokens)
{
	if (tokens.size() == 4 && tokens[2].startsWith("AUTO")) {
		if (tokens[3] == "ON")
			setUIStateForAutoSettings(AutoSettingsList_AutoPairing, AutoSettingsState_ON);
		else if (tokens[3] == "OFF")
			setUIStateForAutoSettings(AutoSettingsList_AutoPairing, AutoSettingsState_OFF);

	} else if (tokens.size() == 3 && tokens[2].startsWith("ENABLE")) {
		if (tokens[2].right(2) == "ON")
			setUIStateForAutoSettings(AutoSettingsList_AutoPairing, AutoSettingsState_ON);
		else if (tokens[2].right(3) == "OFF")
			setUIStateForAutoSettings(AutoSettingsList_AutoPairing, AutoSettingsState_OFF);
	}
}

void AdvancedOptionFrame::on_BUTTON_AUTO_CONNECTION_ON_clicked()
{
	emit signalCommandToServer("$MGT#ENABLE AUTO CONNECTION OFF\n");
}

void AdvancedOptionFrame::on_BUTTON_AUTO_CONNECTION_OFF_clicked()
{
	emit signalCommandToServer("$MGT#ENABLE AUTO CONNECTION ON\n");
}

void AdvancedOptionFrame::on_BUTTON_AUTO_PAIRING_ON_clicked()
{
	emit signalCommandToServer("$MGT#ENABLE AUTO PAIRING OFF\n");
}

void AdvancedOptionFrame::on_BUTTON_AUTO_PAIRING_OFF_clicked()
{
	emit signalCommandToServer("$MGT#ENABLE AUTO PAIRING ON\n");
}

void AdvancedOptionFrame::resizeEvent(QResizeEvent *)
{
	if ((width() != DEFAULT_WIDTH) || (height() != DEFAULT_HEIGHT))
	{
		SetupUI();
	}
}

void AdvancedOptionFrame::SetupUI()
{
	float widthRatio = (float)width() / DEFAULT_WIDTH;
	float heightRatio = (float)height() / DEFAULT_HEIGHT;
	int rx, ry, rw, rh;

	rx = widthRatio * ui->BUTTON_AUTO_CONNECTION_OFF->x();
	ry = heightRatio * ui->BUTTON_AUTO_CONNECTION_OFF->y();
	rw = widthRatio * ui->BUTTON_AUTO_CONNECTION_OFF->width();
	rh = heightRatio * ui->BUTTON_AUTO_CONNECTION_OFF->height();
	ui->BUTTON_AUTO_CONNECTION_OFF->setGeometry(rx, ry, rw, rh);
	ui->BUTTON_AUTO_CONNECTION_ON->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->BUTTON_AUTO_PAIRING_OFF->x();
	ry = heightRatio * ui->BUTTON_AUTO_PAIRING_OFF->y();
	rw = widthRatio * ui->BUTTON_AUTO_PAIRING_OFF->width();
	rh = heightRatio * ui->BUTTON_AUTO_PAIRING_OFF->height();
	ui->BUTTON_AUTO_PAIRING_OFF->setGeometry(rx, ry, rw, rh);
	ui->BUTTON_AUTO_PAIRING_ON->setGeometry(rx, ry, rw, rh);
}
