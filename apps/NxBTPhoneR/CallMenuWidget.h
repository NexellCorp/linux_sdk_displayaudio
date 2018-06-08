#ifndef CALLMENUWIDGET_H
#define CALLMENUWIDGET_H

#include <QWidget>
// for input key
#include <QKeyEvent>
// for loading animation
#include <QMovie>
#include <QListWidgetItem>
#include <QQueue>
#include <QTimer>

#include "defines.h"

#include "BTCommandProcessor.h"

#include "InCommingCallDialog.h"

// for read/parse vCard (*.vcf file format)
#include <io/vCard/VCardReader.h>

#define MAX_RESPONSE_TIME  5000

namespace Ui {
class CallMenuWidget;
}

class CallMenuWidget : public QWidget
{
    Q_OBJECT

signals:
    void signalCommandToServer(QString command);

private slots:
    void slotCommandFromServer(QString command);

    void slotCommandFromServerForInitializing(QString command);

    void slotCommandResponseTimer();

public:
    enum CurrentMenu {
        CurrentMenu_PhoneBook = 0,
        CurrentMenu_Keypad,
        CurrentMenu_Log,
        CurrentMenu_Calling /* Do not use!! */
    };

    enum UIState {
        UIState_DownloadCompleted,
        UIState_Downloading,
        UIState_BluetoothEnabled,
        UIState_BluetoothDisabled
    };

public:
    explicit CallMenuWidget(QWidget *parent = 0);
    ~CallMenuWidget();

private slots:
    void on_BUTTON_PHONEBOOK_MENU_clicked();

    void on_BUTTON_KEYPAD_MENU_clicked();

    void on_BUTTON_LOG_MENU_clicked();

    void on_BUTTON_KEY_1_clicked();

    void on_BUTTON_KEY_2_clicked();

    void on_BUTTON_KEY_3_clicked();

    void on_BUTTON_KEY_4_clicked();

    void on_BUTTON_KEY_5_clicked();

    void on_BUTTON_KEY_6_clicked();

    void on_BUTTON_KEY_7_clicked();

    void on_BUTTON_KEY_8_clicked();

    void on_BUTTON_KEY_9_clicked();

    void on_BUTTON_KEY_0_clicked();

    void on_BUTTON_KEY_BACKSPACE_clicked();

    void on_BUTTON_DIAL_clicked();

    void on_BUTTON_RE_DIAL_clicked();

    void on_BUTTON_SYNC_clicked();

    void on_LISTWIDGET_PHONEBOOK_currentRowChanged(int currentRow);

    void on_LISTWIDGET_PHONEBOOK_DETAIL_itemClicked(QListWidgetItem *item);

//    void on_LISTWIDGET_CALL_LOG_itemClicked(QListWidgetItem *item);

    void on_LISTWIDGET_CALL_LOG_itemDoubleClicked(QListWidgetItem *item);

private:
    void setCurrentMenu(CurrentMenu menu);

    void setUIState(UIState state);

    void inputKey(int key);

    bool updateForBluetoothEnable(QStringList& tokens);

    bool updateForPhoneBook(QStringList& tokens);

    bool updateForCallLog(QStringList& tokens);

    void commandToServer(QString command, QString return_command = QString());

private:
    CurrentMenu m_CurrentMenu;

    BTCommandProcessor* m_pCommandProcessor;

    InCommingCallDialog m_InCommingCallDialog;

    std::vector<VCardReader::VCardProperty> m_PhoneBook;

    std::vector<VCardReader::VCardProperty> m_CallLog;

    // 1st. QString : command
    // 2nd. QString : valid return command
    QQueue<QPair<QString, QString>> m_CommandQueue;

    QMovie* m_pAnimation;

    bool m_bInitialized;

    QTimer m_ResponseTimer;

    UIState m_UIState;

private:
    Ui::CallMenuWidget *ui;
};

#endif // CALLMENUWIDGET_H
