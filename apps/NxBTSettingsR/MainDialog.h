#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QEvent>
#include <QCloseEvent>
#include <QMovie>

// status bar
#include <CNX_StatusBar.h>

// ui dialog
#include "SelectMenuWidget.h"
#include "ConnectionMenuWidget.h"
#include "AdvancedMenuWidget.h"

#include "defines.h"

#ifdef CONFIG_NX_DAUDIO_MANAGER
#   include <NX_Type.h>
#	include <INX_IpcManager.h>
#   include <NX_IpcUtils.h>
#	include <NX_PacpClient.h>

#ifdef CONFIG_TEST_FLAG
extern bool g_first_shown;
#endif

#endif

namespace Ui {
class MainDialog;
}

class MainDialog : public QDialog
{
    Q_OBJECT

private slots:
    void slotCurrentMenuChanged(Menu menu);

	void slotCommandFromServer(QString command);

 public:
    explicit MainDialog(QWidget *parent = 0);
    ~MainDialog();

#ifdef CONFIG_NX_DAUDIO_MANAGER
protected:
	void closeEvent(QCloseEvent *);

	bool eventFilter(QObject *watched, QEvent *event);
#endif

private:
    static void callbackStatusHomeButtonClicked(void* obj);

    static void callbackStatusBackButtonClicked(void* obj);

private:
    void setCurrentMenu(Menu menu);

private:
	CNX_StatusBar* m_pNxStatusBar;

	BTCommandProcessor* m_pCommandProcessor;

    SelectMenuWidget* m_pSelectMenuWidget;

    ConnectionMenuWidget* m_pConnectionMenuWidget;

    AdvancedMenuWidget* m_pAdvancedMenuWidget;

	QMovie* m_pLoadingImage;

private:
    Ui::MainDialog *ui;
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

extern Menu g_current_menu;

#endif // MAINDIALOG_H
