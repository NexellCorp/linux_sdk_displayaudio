#include "CallingMenuWidget.h"
#include "ui_CallingMenuWidget.h"

#include <QDesktopWidget>

#define DEFAULT_WIDTH	1024
#define DEFAULT_HEIGHT	540

CallingMenuWidget::CallingMenuWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CallingMenuWidget)
{
	ui->setupUi(this);

	ResetUI();

	const QRect screen = QApplication::desktop()->screenGeometry();
	if ((width() != screen.width()) || (height() != screen.height()))
	{
		setFixedSize(screen.width(), screen.height() * 0.9);
	}
}

CallingMenuWidget::~CallingMenuWidget()
{
	delete ui;
}

void CallingMenuWidget::slotCommandFromServer(QString command)
{
	int stx = command.indexOf("$");
	int etx = command.indexOf("\n");

	if (stx < 0 || etx < 0) {
		return;
	}

	// body = remove STX and ETX from command
	stx++;
	QString body = command.mid(stx, etx-stx);
	// example) command = "$OK#HS#CALL STATUS#INCOMMING CALL\n"
	// example) body    = "OK#HS#CALL STATUS#INCOMMING CALL"

	QStringList tokens = body.split("#");

	// failure conditions
	if (tokens.size() < 3) {
		return;
	} else if (tokens[0] == "NG") {
		return;
	} else if (tokens[1] != "HS") {
		return;
	}

	// valid commands
	if (tokens[2] == "CALL STATUS") {
		updateCallStatus(tokens);
	} else if (tokens[2] == "AUDIO MUTE STATUS") {
//        ("OK", "HS", "CALL STATUS", "INCOMMING CALL")
//        ("OK", "HS", "AUDIO MUTE STATUS", "AUDIO OPENED", "MICROPHONE MUTE ON")
//        ("OK", "HS", "CALL STATUS", "PICK UP CALL")
		updateAudioMuteStatus(tokens);
	} else if (tokens[2] == "AUDIO OPEN") {
		setAudioDeviceState(AudioDeviceState_Opened, m_UIState == UIState_Calling);
	} else if (tokens[2] == "AUDIO CLOSE") {
		setAudioDeviceState(AudioDeviceState_Closed, m_UIState == UIState_Calling);
	} else if (tokens[2] == "MICROPHONE MUTE ON") {
		setMicrophoneDeviceState(MicrophoneDeviceState_MuteOn, m_UIState == UIState_Calling);
	} else if (tokens[2] == "MICROPHONE MUTE OFF") {
		setMicrophoneDeviceState(MicrophoneDeviceState_MuteOff, m_UIState == UIState_Calling);
	} else if (tokens[2] == "INCOMMING CALL NUMBER") {
//        ("OK", "HS", "INCOMMING CALL NUMBER", " \"0316987429\",129,,,\"      \"")

		updateIncommingCallNumber(tokens);
	} else if (tokens[2].indexOf("DIAL") == 0) {
		updateOutgoingCallNumber(tokens);
	}
}

void CallingMenuWidget::ResetUI()
{
	setUIState(UIState_Init);

	setAudioDeviceState(AudioDeviceState_Closed, m_UIState == UIState_Calling);
	setMicrophoneDeviceState(MicrophoneDeviceState_MuteOn, m_UIState == UIState_Calling);
}

void CallingMenuWidget::on_BUTTON_PICK_UP_CALL_clicked()
{
	emit signalCommandToServer("$HS#PICK UP CALL\n");
}

void CallingMenuWidget::on_BUTTON_HANG_UP_CALL_clicked()
{
	emit signalCommandToServer("$HS#HANG UP CALL\n");
}

void CallingMenuWidget::updateCallStatus(QStringList& tokens)
{
	// example) $OK#HS#CALL STATUS#INCOMMING CALL\n"
	//          {OK, HS, CALL STATUS, INCOMMING CALL}
	if (tokens.size() == 4) {
		if (tokens[3] == "INCOMMING CALL") {
			setUIState(UIState_IncommingCall);
		} else if (tokens[3] == "HANG UP CALL") {
			setUIState(UIState_Init);
		} else if (tokens[3] == "PICK UP CALL") {
			setUIState(UIState_Calling);
		} else if (tokens[3] == "READY OUTGOING CALL") {
			setUIState(UIState_OutGoingCall);
		} else if (tokens[3] == "OUTGOING CALL") {
			setUIState(UIState_OutGoingCall);
		} else if (tokens[3] == "DISCONNECTED CALL") {
			setUIState(UIState_Init);
		} else if (tokens[3] == "UNKNOWN CALL") {
		}
	}
}

void CallingMenuWidget::updateAudioMuteStatus(QStringList& tokens)
{
	// example) "$OK#HS#AUDIO MUTE STATUS#AUDIO OPENED#MICROPHONE MUTE OFF\n"
	//           {OK, HS, AUDIO MUTE STATUS, AUDIO OPENED, MICROPHONE MUTE OFF}
	if (tokens.size() == 5) {
		if (tokens[3] == "AUDIO CLOSED") {
			setAudioDeviceState(AudioDeviceState_Closed, m_UIState == UIState_Calling);
		} else if (tokens[3] == "AUDIO OPENED") {
			setAudioDeviceState(AudioDeviceState_Opened, m_UIState == UIState_Calling);
		}

		if (tokens[4] == "MICROPHONE MUTE ON") {
			setMicrophoneDeviceState(MicrophoneDeviceState_MuteOn, m_UIState == UIState_Calling);
		} else if (tokens[4] == "MICROPHONE MUTE OFF") {
			setMicrophoneDeviceState(MicrophoneDeviceState_MuteOff, m_UIState == UIState_Calling);
		}
	}
}

void CallingMenuWidget::updateIncommingCallNumber(QStringList& tokens)
{
	// example) _"0316987429",129,,,"______"
	if (tokens.size() == 4) {
		QStringList subTokens = tokens[3].split(",");
		if (subTokens.size() > 0) {
			int start, position;
			QString callNumber;

			start = subTokens[0].indexOf('"');
			if (start < 0)
				return;

			position = subTokens[0].indexOf('"', ++start);
			if (position < 0)
				return;

			callNumber = subTokens[0].mid(start, position-start);

			ui->LABEL_CALL_NUMBER->setText(callNumber);
		}
	}
}

void CallingMenuWidget::updateOutgoingCallNumber(QStringList& tokens)
{
	if (tokens.size() == 3) {
		// -5 is length for "DIAL_"
		QString callNumber = tokens[2].right(tokens[2].length()-5);
		ui->LABEL_CALL_NUMBER->setText(callNumber);
	}
}

void CallingMenuWidget::setUIState(UIState state)
{
	if (state == m_UIState)
		return;

	m_UIState = state;	

	QRect rect;
	switch (state) {
	case UIState_Init:
		ui->LABEL_CALL_NUMBER->clear();
		break;

	case UIState_IncommingCall:
		ui->BUTTON_HANG_UP_CALL->show();
		ui->BUTTON_PICK_UP_CALL->show();

		ui->BUTTON_HANG_UP_CALL->setText("Reject");
		ui->BUTTON_PICK_UP_CALL->setText("Accept");

		ui->BUTTON_PICK_UP_CALL->move(ui->BUTTON_AUDIO_OPEN->x() + ui->BUTTON_AUDIO_OPEN->width()/2-ui->BUTTON_HANG_UP_CALL->width()/2, ui->BUTTON_HANG_UP_CALL->y());
		ui->BUTTON_HANG_UP_CALL->move(ui->BUTTON_MICROPHONE_MUTE_ON->x() + ui->BUTTON_MICROPHONE_MUTE_ON->width()/2-ui->BUTTON_PICK_UP_CALL->width()/2, ui->BUTTON_PICK_UP_CALL->y());

		ui->BUTTON_MICROPHONE_MUTE_OFF->hide();
		ui->BUTTON_MICROPHONE_MUTE_ON->hide();
		ui->BUTTON_AUDIO_OPEN->hide();
		ui->BUTTON_AUDIO_CLOSE->hide();
		break;

	case UIState_OutGoingCall:
		ui->BUTTON_HANG_UP_CALL->show();
		ui->BUTTON_PICK_UP_CALL->hide();

		ui->BUTTON_HANG_UP_CALL->setText("Hang Up");
		rect = QApplication::desktop()->screenGeometry();
		ui->BUTTON_HANG_UP_CALL->move(rect.width() / 2 - ui->BUTTON_HANG_UP_CALL->width() / 2, ui->BUTTON_HANG_UP_CALL->y());

		ui->BUTTON_MICROPHONE_MUTE_OFF->hide();
		ui->BUTTON_MICROPHONE_MUTE_ON->hide();
		ui->BUTTON_AUDIO_OPEN->hide();
		ui->BUTTON_AUDIO_CLOSE->hide();
		break;

	case UIState_Calling:
		ui->BUTTON_HANG_UP_CALL->show();
		ui->BUTTON_PICK_UP_CALL->hide();

		ui->BUTTON_HANG_UP_CALL->setText("Hang Up");
		rect = QApplication::desktop()->screenGeometry();
		ui->BUTTON_HANG_UP_CALL->move(rect.width() / 2 - ui->BUTTON_HANG_UP_CALL->width() / 2, ui->BUTTON_HANG_UP_CALL->y());

		setAudioDeviceState(m_AudioDeviceState, m_UIState == UIState_Calling);
		setMicrophoneDeviceState(m_MicrophoneDeviceState, m_UIState == UIState_Calling);
		break;
	}
}

void CallingMenuWidget::setAudioDeviceState(AudioDeviceState state, bool ui_update_on/*= true*/)
{
	m_AudioDeviceState = state;

	if (!ui_update_on)
		return;

	switch (state) {
	case AudioDeviceState_Opened:
		ui->BUTTON_AUDIO_CLOSE->show();
		ui->BUTTON_AUDIO_OPEN->hide();
		break;

	case AudioDeviceState_Closed:
		ui->BUTTON_AUDIO_OPEN->show();
		ui->BUTTON_AUDIO_CLOSE->hide();
		break;
	}
}

void CallingMenuWidget::setMicrophoneDeviceState(MicrophoneDeviceState state, bool ui_update_on/*= true*/)
{
	m_MicrophoneDeviceState = state;

	if (!ui_update_on)
		return;

	switch (state) {
	case MicrophoneDeviceState_MuteOn:
		ui->BUTTON_MICROPHONE_MUTE_OFF->show();
		ui->BUTTON_MICROPHONE_MUTE_ON->hide();
		break;

	case MicrophoneDeviceState_MuteOff:
		ui->BUTTON_MICROPHONE_MUTE_ON->show();
		ui->BUTTON_MICROPHONE_MUTE_OFF->hide();
		break;
	}
}

void CallingMenuWidget::on_BUTTON_MICROPHONE_MUTE_OFF_clicked()
{
	emit signalCommandToServer("$HS#MICROPHONE MUTE OFF\n");
}

void CallingMenuWidget::on_BUTTON_MICROPHONE_MUTE_ON_clicked()
{
	emit signalCommandToServer("$HS#MICROPHONE MUTE ON\n");
}

void CallingMenuWidget::on_BUTTON_AUDIO_CLOSE_clicked()
{
	emit signalCommandToServer("$HS#AUDIO CLOSE\n");
}

void CallingMenuWidget::on_BUTTON_AUDIO_OPEN_clicked()
{
	emit signalCommandToServer("$HS#AUDIO OPEN\n");
}

void CallingMenuWidget::resizeEvent(QResizeEvent *)
{
	if ((width() != DEFAULT_WIDTH) || (height() != DEFAULT_HEIGHT))
	{
		SetupUI();
	}
}

void CallingMenuWidget::SetupUI()
{
	float widthRatio = (float)width() / DEFAULT_WIDTH;
	float heightRatio = (float)height() / DEFAULT_HEIGHT;
	int rx, ry, rw, rh;

	rx = widthRatio * ui->LABEL_CALL_NUMBER->x();
	ry = heightRatio * ui->LABEL_CALL_NUMBER->y();
	rw = widthRatio * ui->LABEL_CALL_NUMBER->width();
	rh = heightRatio * ui->LABEL_CALL_NUMBER->height();
	ui->LABEL_CALL_NUMBER->setGeometry(rx, ry, rw, rh);

	// sync
	rx = widthRatio * ui->BUTTON_AUDIO_CLOSE->x();
	ry = heightRatio * ui->BUTTON_AUDIO_CLOSE->y();
	rw = widthRatio * ui->BUTTON_AUDIO_CLOSE->width();
	rh = heightRatio * ui->BUTTON_AUDIO_CLOSE->height();
	ui->BUTTON_AUDIO_CLOSE->setGeometry(rx, ry, rw, rh);
	ui->BUTTON_AUDIO_OPEN->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->BUTTON_MICROPHONE_MUTE_OFF->x();
	ry = heightRatio * ui->BUTTON_MICROPHONE_MUTE_OFF->y();
	rw = widthRatio * ui->BUTTON_MICROPHONE_MUTE_OFF->width();
	rh = heightRatio * ui->BUTTON_MICROPHONE_MUTE_OFF->height();
	ui->BUTTON_MICROPHONE_MUTE_OFF->setGeometry(rx, ry, rw, rh);
	ui->BUTTON_MICROPHONE_MUTE_ON->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->BUTTON_PICK_UP_CALL->x();
	ry = heightRatio * ui->BUTTON_PICK_UP_CALL->y();
	rw = widthRatio * ui->BUTTON_PICK_UP_CALL->width();
	rh = heightRatio * ui->BUTTON_PICK_UP_CALL->height();
	ui->BUTTON_PICK_UP_CALL->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->BUTTON_HANG_UP_CALL->x();
	ry = heightRatio * ui->BUTTON_HANG_UP_CALL->y();
	rw = widthRatio * ui->BUTTON_HANG_UP_CALL->width();
	rh = heightRatio * ui->BUTTON_HANG_UP_CALL->height();
	ui->BUTTON_HANG_UP_CALL->setGeometry(rx, ry, rw, rh);
}
