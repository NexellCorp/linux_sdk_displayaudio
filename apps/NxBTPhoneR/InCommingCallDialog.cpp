#include "InCommingCallDialog.h"
#include "ui_InCommingCallDialog.h"
#include <QDesktopWidget>

InCommingCallDialog::InCommingCallDialog(QWidget *parent) :
    QDialog(parent, Qt::FramelessWindowHint),
    ui(new Ui::InCommingCallDialog)
{
    ui->setupUi(this);

	reset();
}

InCommingCallDialog::~InCommingCallDialog()
{
    delete ui;
}

void InCommingCallDialog::reset()
{
	setUIState(UIState_IncommingCall);

	setAudioDeviceState(AudioDeviceState_Closed, m_UIState == UIState_Calling);
	setMicrophoneDeviceState(MicrophoneDeviceState_MuteOn, m_UIState == UIState_Calling);
}

void InCommingCallDialog::slotCommandFromServer(QString command)
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

void InCommingCallDialog::on_BUTTON_HANG_UP_CALL_clicked()
{
    emit signalCommandToServer("$HS#HANG UP CALL\n");
}

void InCommingCallDialog::on_BUTTON_PICK_UP_CALL_clicked()
{
    emit signalCommandToServer("$HS#PICK UP CALL\n");
}

void InCommingCallDialog::updateCallStatus(QStringList& tokens)
{
    // example) $OK#HS#CALL STATUS#INCOMMING CALL\n"
    //          {OK, HS, CALL STATUS, INCOMMING CALL}
    if (tokens.size() == 4) {
        if (tokens[3] == "INCOMMING CALL") {
            setUIState(UIState_IncommingCall);
        } else if (tokens[3] == "HANG UP CALL") {
        } else if (tokens[3] == "PICK UP CALL") {
            setUIState(UIState_Calling);
        } else if (tokens[3] == "READY OUTGOING CALL") {
            setUIState(UIState_OutGoingCall);
        } else if (tokens[3] == "OUTGOING CALL") {
            setUIState(UIState_OutGoingCall);
        } else if (tokens[3] == "DISCONNECTED CALL") {
        } else if (tokens[3] == "UNKNOWN CALL") {
        }
    }
}

void InCommingCallDialog::updateAudioMuteStatus(QStringList& tokens)
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

void InCommingCallDialog::updateIncommingCallNumber(QStringList& tokens)
{
    // example) _"0316987429",129,,,"______"
    if (tokens.size() == 4) {
        QStringList subTokens = tokens[3].split(",");
        if (subTokens.size() == 5) {
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

void InCommingCallDialog::updateOutgoingCallNumber(QStringList& tokens)
{
    if (tokens.size() == 3) {
        // -5 is length for "DIAL_"
        QString callNumber = tokens[2].right(tokens[2].length()-5);
        ui->LABEL_CALL_NUMBER->setText(callNumber);
    }
}

void InCommingCallDialog::setUIState(UIState state)
{
    if (state == m_UIState)
        return;

	m_UIState = state;

    QRect rect;
    switch (state) {
    case UIState_IncommingCall:
        ui->BUTTON_HANG_UP_CALL->show();
        ui->BUTTON_PICK_UP_CALL->show();

        ui->BUTTON_HANG_UP_CALL->setText("Reject");
        ui->BUTTON_PICK_UP_CALL->setText("Accept");

        ui->BUTTON_HANG_UP_CALL->move(270, 330);
        ui->BUTTON_PICK_UP_CALL->move(590, 300);

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

void InCommingCallDialog::setAudioDeviceState(AudioDeviceState state, bool ui_update_on/*= true*/)
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

void InCommingCallDialog::setMicrophoneDeviceState(MicrophoneDeviceState state, bool ui_update_on/*= true*/)
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

void InCommingCallDialog::on_BUTTON_MICROPHONE_MUTE_OFF_clicked()
{
	emit signalCommandToServer("$HS#MICROPHONE MUTE OFF\n");
}

void InCommingCallDialog::on_BUTTON_MICROPHONE_MUTE_ON_clicked()
{
	emit signalCommandToServer("$HS#MICROPHONE MUTE ON\n");
}

void InCommingCallDialog::on_BUTTON_AUDIO_OPEN_clicked()
{
	emit signalCommandToServer("$HS#AUDIO OPEN\n");
}

void InCommingCallDialog::on_BUTTON_AUDIO_CLOSE_clicked()
{
	emit signalCommandToServer("$HS#AUDIO CLOSE\n");
}
