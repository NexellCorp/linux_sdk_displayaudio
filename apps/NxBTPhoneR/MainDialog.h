#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QShowEvent>
#include <QHideEvent>
#include <QCloseEvent>

#include "BTCommandProcessor.h"

#include "SelectMenuWidget.h"
#include "CallMenuWidget.h"
#include "MessageMenuWidget.h"
#include "InCommingCallDialog.h"

#include "NxEvent.h"

#include "LogUtility.hpp"

#include <CNX_StatusBar.h>

#include <pthread.h>

#include <QElapsedTimer>
#include <QTimer>

#include <NX_PacpClient.h>

#ifdef CONFIG_NX_DAUDIO_MANAGER
#   include <NX_Type.h>
#	include <INX_IpcManager.h>
#   include <NX_IpcPacket.h>
#   include <NX_IpcUtils.h>
#endif

namespace Ui {
class MainDialog;
}

class MainDialog : public QDialog
{
    Q_OBJECT

private slots:
    void slotCommandFromServer(QString command);

    void slotCurrentMenuChanged(Menu menu);

    void slotRetryCommandTimer();

public:
    explicit MainDialog(QWidget *parent = 0);
    ~MainDialog();

#ifdef CONFIG_NX_DAUDIO_MANAGER
protected:
    bool eventFilter(QObject *watched, QEvent *event);

//    void showEvent(QShowEvent *);

//    void hideEvent(QHideEvent *event);

	void closeEvent(QCloseEvent *);
#endif
private:
    // NX STATUS BAR - [Home] button clicked
    static void callbackStatusHomeButtonClicked(void* obj);

    // NX STATUS BAR - [Back] button clicked
    static void callbackStatusBackButtonClicked(void* obj);

    // change current menu
    void setCurrentMenu(Menu menu);

	//
    void processForCallDisconnected();

private:
    SelectMenuWidget* m_pSelectMenuWidget;

    CallMenuWidget* m_pCallMenuWidget;

    MessageMenuWidget* m_pMessageMenuWidget;

    InCommingCallDialog* m_pCallingDialog;

    BTCommandProcessor* m_pBTCommandProcessor;

	CNX_StatusBar* m_pNxStatusBar;

    // 전화 연결이 끊어졌을 경우에 대한 판단하는 플래그
    // 해당 플래그가 켜진 상태에서 $HS#AUDIO STATUS#AUDIO CLOSED#... 명령어 수신 시, 오디오 포커스를 풀어주기 위한 목적
    bool m_bDisconnectedCall;

    bool m_bAudioClosedForHS;

	bool m_bBTConnectedForHS;

    QElapsedTimer m_TimeChecker;

    int m_nRetryCommandCount;
    QString m_RetryCommand;
    QTimer m_RetryCommandTimer;

private:
    Ui::MainDialog *ui;
};

extern Menu g_current_menu, g_previous_menu;
extern bool g_calling_end_is_exit;
#ifdef CONFIG_TEST_FLAG
extern bool g_first_shown;
#endif

#endif // MAINDIALOG_H
