#ifndef MESSAGEMENUWIDGET_H
#define MESSAGEMENUWIDGET_H

#include <QDialog>
#include <QMovie>

#include "defines.h"

namespace Ui {
class MessageMenuWidget;
}

class MessageMenuWidget : public QWidget
{
    Q_OBJECT

signals:
    void signalCommandToServer(QString command);

private slots:
    void slotCommandFromServer(QString command);

public:
    enum UIState {
        UIState_DownloadCompleted,
        UIState_Downloading,
        UIState_BluetoothEnabled,
        UIState_BluetoothDisabled
    };

public:
    explicit MessageMenuWidget(QWidget *parent = 0);
    ~MessageMenuWidget();

protected:
	void resizeEvent(QResizeEvent *event);

private slots:
    void on_BUTTON_SYNC_clicked();

private:
	void SetupUI();

    void setUIState(UIState state);

    bool updateForBluetoothEnable(QStringList& tokens);

    bool updateForSMSMessage(QStringList& tokens);

private:
    QMovie* m_pAnimation;

private:
    Ui::MessageMenuWidget *ui;
};

#endif // MESSAGEMENUWIDGET_H
