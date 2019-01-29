#ifndef CONNECTIONMENUFRAME_H
#define CONNECTIONMENUFRAME_H

#include <QFrame>
#include <QMouseEvent>
#include "BTPairedDeviceItem.h"

namespace Ui {
class ConnectionMenuFrame;
}

class ConnectionMenuFrame : public QFrame
{
	Q_OBJECT

signals:
	void signalCommandToServer(QString command);

private slots:
	void slotCommandFromServer(QString command);

	void slotEnterFromKeyboard();

	void slotExitFromKeyboard();

	void slotRejected();

	void slotAVKConnectButtonClicked(int, BTPairedDeviceItem*);

	void slotHSConnectButtonClicked(int, BTPairedDeviceItem*);

public:
	enum UIState {
		UIState_Initializing,
		UIState_Initialized,
	};

	explicit ConnectionMenuFrame(QWidget *parent = 0);
	~ConnectionMenuFrame();

protected:
	bool eventFilter(QObject *, QEvent *);

	void mousePressEvent(QMouseEvent *);

	void resizeEvent(QResizeEvent *event);

private slots:
	void on_BUTTON_RENAME_BT_DEVICE_clicked();

	void on_BUTTON_UNPAIR_clicked();

	void on_BUTTON_UNPAIR_ALL_clicked();

private:
	void SetupUI();

	void setUIState(UIState state);

	void updateToUIForLocalDeviceName(QStringList& tokens);

	void updateToUIForLocalDeviceAddress(QStringList& tokens);

	void updateToUIForPairedDeviceList(QStringList& tokens);

//	void updateToUIForPairingRequest(QStringList& tokens);

	void updateToUIForConnectionStatus(QStringList& tokens);

	void updateToUIForConnectedDeviceIndex(QStringList& tokens);

//	void updateToUIForPairFailed(QStringList& tokens);

	void updateForAutoPairing(QStringList& tokens);

private:
	UIState m_UIState;

	bool m_bBTServiceConnected;

	bool m_bAutoPairing;

	QPoint m_KeyboardPt;

private:
	Ui::ConnectionMenuFrame *ui;
};

#endif // CONNECTIONMENUFRAME_H
