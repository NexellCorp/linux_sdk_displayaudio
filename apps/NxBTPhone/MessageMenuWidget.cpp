#include "MessageMenuWidget.h"
#include "ui_MessageMenuWidget.h"
#include <QDesktopWidget>

#define DEFAULT_WIDTH	1024
#define DEFAULT_HEIGHT	540

MessageMenuWidget::MessageMenuWidget(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint),
    ui(new Ui::MessageMenuWidget)
{
    ui->setupUi(this);

	const QRect screen = QApplication::desktop()->screenGeometry();
	if ((width() != screen.width()) || (height() != screen.height()))
	{
		setFixedSize(screen.width(), screen.height() * 0.9);
	}
}

MessageMenuWidget::~MessageMenuWidget()
{
    delete ui;
}

void MessageMenuWidget::slotCommandFromServer(QString command)
{
    int stx = command.indexOf("$");
    int etx = command.lastIndexOf("\n");

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

		ui->EDIT_SMS_MESSAGE->clear();
		ui->EDIT_SMS_MESSAGE->appendPlainText(text);

        return true;
    }

    return false;
}

void MessageMenuWidget::on_BUTTON_SYNC_clicked()
{
    emit signalCommandToServer("$MCE#DOWNLOAD SMS MESSAGE\n");
}

void MessageMenuWidget::resizeEvent(QResizeEvent *)
{
	if ((width() != DEFAULT_WIDTH) || (height() != DEFAULT_HEIGHT))
	{
		SetupUI();
	}
}

void MessageMenuWidget::SetupUI()
{
	float widthRatio = (float)width() / DEFAULT_WIDTH;
	float heightRatio = (float)height() / DEFAULT_HEIGHT;
	int rx, ry, rw, rh;

	rx = widthRatio * ui->BUTTON_SYNC->x();
	ry = heightRatio * ui->BUTTON_SYNC->y();
	rw = widthRatio * ui->BUTTON_SYNC->width();
	rh = heightRatio * ui->BUTTON_SYNC->height();
	ui->BUTTON_SYNC->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->LABEL_LOADING->x();
	ry = heightRatio * ui->LABEL_LOADING->y();
	rw = widthRatio * ui->LABEL_LOADING->width();
	rh = heightRatio * ui->LABEL_LOADING->height();
	ui->LABEL_LOADING->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->EDIT_SMS_MESSAGE->x();
	ry = heightRatio * ui->EDIT_SMS_MESSAGE->y();
	rw = widthRatio * ui->EDIT_SMS_MESSAGE->width();
	rh = heightRatio * ui->EDIT_SMS_MESSAGE->height();
	ui->EDIT_SMS_MESSAGE->setGeometry(rx, ry, rw, rh);
}
