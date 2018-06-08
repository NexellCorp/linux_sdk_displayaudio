#ifndef CONNECTIONMENUWIDGET_H
#define CONNECTIONMENUWIDGET_H

#include <QWidget>
#include <QListWidgetItem>

#include "BTCommandProcessor.h"
#include "MessageDialog.h"
#include "BTPairedDeviceItem.h"

#include "defines.h"

#include <Keyboard/KeyboardDialog.h>

namespace Ui {
class ConnectionMenuWidget;
}

class ConnectionMenuWidget : public QWidget
{
    Q_OBJECT

signals:
    void signalCommandToServer(QString command);

private slots:
    void slotCommandFromServer(QString command);

    void slotPairingRequestAccepted();

    void slotPairingRequestRejected();

	void slotAVKConnectButtonClicked(int row, BTPairedDeviceItem* custom);

	void slotHSConnectButtonClicked(int row, BTPairedDeviceItem* custom);

public:
    enum UIState {
        UIState_Initializing,
        UIState_Initialized,
    };

public:
    explicit ConnectionMenuWidget(QWidget *parent = 0);
    ~ConnectionMenuWidget();

private:
    void setUIState(UIState state);

    void updateToUIForLocalDeviceName(QStringList& tokens);

    void updateToUIForLocalDeviceAddress(QStringList& tokens);

    void updateToUIForPairedDeviceList(QStringList& tokens);

    void updateToUIForPairingRequest(QStringList& tokens);

	void updateToUIForConnectionStatus(QStringList& tokens);

	void updateToUIForConnectedDeviceIndex(QStringList& tokens);

    void updateToUIForPairFailed(QStringList& tokens);

    void updateForAutoPairing(QStringList& tokens);

private slots:
    void on_BUTTON_UNPAIR_clicked();

    void on_BUTTON_UNPAIR_ALL_clicked();

    void on_BUTTON_RENAME_BT_DEVICE_clicked();

private:
    UIState m_UIState;

    BTCommandProcessor* m_pCommandProcessor;

    MessageDialog* m_pPairingRequestDialog;

    bool m_bAutoPairing;

private:
    Ui::ConnectionMenuWidget *ui;
};

extern Menu g_current_menu;

#endif // CONNECTIONMENUWIDGET_H
