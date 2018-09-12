#ifndef CALLINGMENUWIDGET_H
#define CALLINGMENUWIDGET_H

#include <QWidget>

namespace Ui {
class CallingMenuWidget;
}

class CallingMenuWidget : public QWidget
{
	Q_OBJECT

signals:
	void signalCommandToServer(QString command);

private slots:
	void slotCommandFromServer(QString command);

	void on_BUTTON_PICK_UP_CALL_clicked();

	void on_BUTTON_HANG_UP_CALL_clicked();

	void on_BUTTON_MICROPHONE_MUTE_OFF_clicked();

	void on_BUTTON_MICROPHONE_MUTE_ON_clicked();

	void on_BUTTON_AUDIO_CLOSE_clicked();

	void on_BUTTON_AUDIO_OPEN_clicked();

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

	explicit CallingMenuWidget(QWidget *parent = 0);
	~CallingMenuWidget();

	void ResetUI();

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
	Ui::CallingMenuWidget *ui;
};

#endif // CALLINGMENUWIDGET_H
