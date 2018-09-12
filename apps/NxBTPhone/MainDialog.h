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

#include <CNX_StatusBar.h>

#include <pthread.h>

#include <QElapsedTimer>
#include <QTimer>

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
