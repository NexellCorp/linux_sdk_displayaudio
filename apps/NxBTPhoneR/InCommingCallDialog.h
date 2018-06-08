#ifndef INCOMMINGCALLDIALOG_H
#define INCOMMINGCALLDIALOG_H

#include <QDialog>
#include "defines.h"

namespace Ui {
class InCommingCallDialog;
}

class InCommingCallDialog : public QDialog
{
    Q_OBJECT

signals:
    void signalCommandToServer(QString command);

private slots:
    void slotCommandFromServer(QString command);

public:
    enum UIState {
        UIState_IncommingCall,
        UIState_OutGoingCall,
        UIState_Calling
    };

	enum AudioDeviceState {
		AudioDeviceState_Opened,
		AudioDeviceState_Closed
	};

	enum MicrophoneDeviceState {
		MicrophoneDeviceState_MuteOn,
		MicrophoneDeviceState_MuteOff
	};

public:
    explicit InCommingCallDialog(QWidget *parent = 0);
    ~InCommingCallDialog();

	void reset();

private slots:
    void on_BUTTON_HANG_UP_CALL_clicked();

    void on_BUTTON_PICK_UP_CALL_clicked();

	void on_BUTTON_MICROPHONE_MUTE_OFF_clicked();

	void on_BUTTON_MICROPHONE_MUTE_ON_clicked();

	void on_BUTTON_AUDIO_OPEN_clicked();

	void on_BUTTON_AUDIO_CLOSE_clicked();

private:
    void updateCallStatus(QStringList& tokens);

    void updateAudioMuteStatus(QStringList& tokens);

    void updateIncommingCallNumber(QStringList& tokens);

    void updateOutgoingCallNumber(QStringList& tokens);

private:
    void setUIState(UIState state);

	void setAudioDeviceState(AudioDeviceState state, bool ui_update_on = true);

	void setMicrophoneDeviceState(MicrophoneDeviceState state, bool ui_update_on = true);

private:
    UIState m_UIState;

	AudioDeviceState m_AudioDeviceState;

	MicrophoneDeviceState m_MicrophoneDeviceState;

private:
    Ui::InCommingCallDialog *ui;
};

extern Nexell::BTPhone::Menu g_current_menu, g_previous_menu;

#endif // INCOMMINGCALLDIALOG_H
