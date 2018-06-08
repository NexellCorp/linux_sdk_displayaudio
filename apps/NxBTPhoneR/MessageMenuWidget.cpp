#include "MessageMenuWidget.h"
#include "ui_MessageMenuWidget.h"


MessageMenuWidget::MessageMenuWidget(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint),
    ui(new Ui::MessageMenuWidget)
{
    ui->setupUi(this);
}

MessageMenuWidget::~MessageMenuWidget()
{
    delete ui;
}

void MessageMenuWidget::slotCommandFromServer(QString command)
{
    int stx = command.indexOf("$");
    int etx = command.indexOf("\n");

    if (stx < 0 || etx < 0) {
        return;
    }

    // body = remove STX and ETX from command
    stx++;
    QString body = command.mid(stx, etx-stx);

    // example) command = "$OK#MCE#IS CONNECTED#CONNECTED\n"
    // example) body    = "OK#MCE#IS CONNECTED#CONNECTED"

    QStringList tokens = body.split("#");

    // failure conditions
    if (tokens.size() < 3) {
        return;
    } else if (tokens[0] == "NG") {
        return;
    } else if (tokens[1] != "MCE") {
        return;
    }

    if (tokens[1] == "MCE") {
        if (tokens[2] == "IS CONNECTED") {
            updateForBluetoothEnable(tokens);
        } else if (tokens[2] == "CONNECTION STATUS") {
            updateForBluetoothEnable(tokens);
        } else if (tokens[2] == "DOWNLOAD SMS MESSAGE") {
            updateForSMSMessage(tokens);
        }
    }
}

void MessageMenuWidget::setUIState(UIState state)
{
    switch (state) {
    case UIState_DownloadCompleted:
        ui->BUTTON_SYNC->show();
        ui->LABEL_LOADING->hide();
        m_pAnimation->stop();
        break;

    case UIState_Downloading:
        m_pAnimation->start();
        ui->LABEL_LOADING->show();
        ui->BUTTON_SYNC->hide();
        break;

    case UIState_BluetoothEnabled:
        ui->BUTTON_SYNC->setEnabled(true);
        break;

    case UIState_BluetoothDisabled:
        ui->BUTTON_SYNC->setEnabled(false);
        break;
    }
}

bool MessageMenuWidget::updateForBluetoothEnable(QStringList& tokens)
{
    if (tokens.size() >= 4) {
        if (tokens[3] == "CONNECTED") {
            setUIState(UIState_BluetoothEnabled);
            return true;
        } else {
            setUIState(UIState_BluetoothDisabled);
            return false;
        }
    }

    return false;
}

bool MessageMenuWidget::updateForSMSMessage(QStringList &tokens)
{
    // OK, MCE, DOWNLOAD SMS MESSAGE, [name], [phonenumber], [message]
    if (tokens.size() == 6) {
        QString text;

        text = tokens[3];
        for (int i = 4; i < 6; i++)
            text += "\n" + tokens[i];

        ui->LISTWIDGET_SMS_MESSAGE->addItem(text);

        return true;
    }

    return false;
}

void MessageMenuWidget::on_BUTTON_SYNC_clicked()
{
    ui->LISTWIDGET_SMS_MESSAGE->clear();
    emit signalCommandToServer("$MCE#DOWNLOAD SMS MESSAGE\n");
}
