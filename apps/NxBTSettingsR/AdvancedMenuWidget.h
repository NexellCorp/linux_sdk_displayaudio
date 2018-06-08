#ifndef ADVANCEDMENUWIDGET_H
#define ADVANCEDMENUWIDGET_H

#include <QWidget>
#include "BTCommandProcessor.h"

#include "defines.h"

namespace Ui {
class AdvancedMenuWidget;
}

class AdvancedMenuWidget : public QWidget
{
    Q_OBJECT

signals:
    void signalCommandToServer(QString command);

private slots:
    void slotCommandFromServer(QString command);

    void on_BUTTON_AUTO_CONNECTION_OFF_clicked();

    void on_BUTTON_AUTO_CONNECTION_ON_clicked();

    void on_BUTTON_AUTO_PAIRING_OFF_clicked();

    void on_BUTTON_AUTO_PAIRING_ON_clicked();

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

public:
    explicit AdvancedMenuWidget(QWidget *parent = 0);
    ~AdvancedMenuWidget();

private:
    void setUIState(UIState state);

    void setUIStateForAutoSettings(AutoSettingsList type, AutoSettingsState state);

    void updateToUIForAutoConnection(QStringList& tokens);

    void updateToUIForAutoPairing(QStringList& tokens);

private:
    UIState m_UIState;

    AutoSettingsState m_AutoConnectionState;

    AutoSettingsState m_AutoPairingState;

    BTCommandProcessor* m_pCommandProcessor;

private:
    Ui::AdvancedMenuWidget *ui;
};

extern Menu g_current_menu;

#endif // ADVANCEDMENUWIDGET_H
