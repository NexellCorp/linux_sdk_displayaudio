#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

#include "BTCommandProcessor.h"

#ifdef CONFIG_NX_DAUDIO_MANAGER
#   include <QEvent>
#   include <QShowEvent>
#   include <QHideEvent>
#   include <QCloseEvent>

#   include <NX_Type.h>
#   include <INX_IpcManager.h>
#   include <NX_IpcPacket.h>
#   include <NX_IpcUtils.h>

#ifdef CONFIG_TEST_FLAG
extern bool g_first_shown;
#endif

#endif

#include <CNX_StatusBar.h>
#include <NX_PacpClient.h>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

signals:
    void signalCommandToServer(QString command);

private slots:
    void slotCommandProcessorEnabled();
    void slotCommandFromServer(QString command);

public:
    enum UIState {
        UIState_Playing,
        UIState_Paused,
        UIState_Stopped,
        UIState_BluetoothEnabled,
        UIState_BluetoothDisabled
    };

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

#ifdef CONFIG_NX_DAUDIO_MANAGER
protected:
    bool eventFilter(QObject *watched, QEvent *event);

//    void showEvent(QShowEvent *);

//    void hideEvent(QHideEvent *event);

    void closeEvent(QCloseEvent *event);
#endif
private slots:
    void on_BUTTON_PLAY_PREV_clicked();

    void on_BUTTON_PLAY_NEXT_clicked();

    void on_BUTTON_PLAY_START_clicked();

    void on_BUTTON_PLAY_PAUSE_clicked();

private:
    bool updateToUIForBluetoothEnable(QStringList& tokens);

    void updateToUIForMediaElements(QStringList& tokens);

    void updateToUIForPlayPosition(QStringList& tokens);

    void updateToUIForPlayStatus(QStringList& tokens);

    void updateToUIForPlayInformation(QStringList& tokens);

    void setUIState(UIState state);

    static void callbackStatusHomeButtonClicked(void* obj);

    static void callbackStatusBackButtonClicked(void* obj);

private:
    BTCommandProcessor* m_pCommandProcessor;

    UIState m_UIState;

	CNX_StatusBar* m_pNxStatusBar;

private:
    Ui::Dialog *ui;
};

#ifdef CONFIG_NX_DAUDIO_MANAGER
class NxEvent : QEvent
{
public:
    int32_t m_iEventType;
    QString m_szData;

    NxEvent(QEvent::Type type) : QEvent(type)
    {
    }
};
#endif

#endif // DIALOG_H
