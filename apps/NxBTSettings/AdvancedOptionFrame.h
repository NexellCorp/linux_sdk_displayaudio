#ifndef ADVANCEDOPTIONFRAME_H
#define ADVANCEDOPTIONFRAME_H

#include <QFrame>
#include <unistd.h>

namespace Ui {
class AdvancedOptionFrame;
}

class AdvancedOptionFrame : public QFrame
{
	Q_OBJECT

signals:
	void signalCommandToServer(QString command);

private slots:
	void slotCommandFromServer(QString command);

	void on_BUTTON_AUTO_CONNECTION_ON_clicked();

	void on_BUTTON_AUTO_CONNECTION_OFF_clicked();

	void on_BUTTON_AUTO_PAIRING_ON_clicked();

	void on_BUTTON_AUTO_PAIRING_OFF_clicked();

public:
	enum UIState {
		UIState_Initializing,
		UIState_Initialized,
	};

	enum AutoSettingsState {
		AutoSettingsState_ON,
		AutoSettingsState_OFF
	};

	enum AutoSettingsList {
		AutoSettingsList_AutoConnection,
		AutoSettingsList_AutoPairing
	};


	explicit AdvancedOptionFrame(QWidget *parent = 0);
	~AdvancedOptionFrame();

private:
	void setUIState(UIState state);

	void setUIStateForAutoSettings(AutoSettingsList type, AutoSettingsState state);

	void updateToUIForAutoConnection(QStringList& tokens);

	void updateToUIForAutoPairing(QStringList& tokens);

private:
	UIState m_UIState;

	AutoSettingsState m_AutoConnectionState;

	AutoSettingsState m_AutoPairingState;

private:
	Ui::AdvancedOptionFrame *ui;
};

#endif // ADVANCEDOPTIONFRAME_H
