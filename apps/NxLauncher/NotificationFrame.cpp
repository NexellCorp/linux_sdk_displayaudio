#include "NotificationFrame.h"
#include "ui_NotificationFrame.h"

NotificationFrame::NotificationFrame(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::NotificationFrame)
{
	ui->setupUi(this);

	m_uiTimeout = 0;

	connect(&m_Timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
}

NotificationFrame::~NotificationFrame()
{
	delete ui;
}

void NotificationFrame::SetRequestor(QString requestor)
{
	m_Requestor = requestor;
}

QString NotificationFrame::GetRequestor()
{
	return m_Requestor;
}

void NotificationFrame::SetButtonVisibility(ButtonVisibility eVisibility)
{
	switch (eVisibility) {
	case ButtonVisibility_Ok:
		ui->BUTTON_OK->show();
		ui->BUTTON_CANCEL->hide();
		break;

	case ButtonVisibility_Cencel:
		ui->BUTTON_CANCEL->show();
		ui->BUTTON_OK->hide();
		break;

	case ButtonVisibility_Default:
		ui->BUTTON_OK->show();
		ui->BUTTON_CANCEL->show();
		break;

	default: break;
	}
}

void NotificationFrame::SetButonStyleSheet(ButtonType eType, QString styleSheet)
{
	switch (eType) {
	case ButtonType_Ok:
	{
		if (!styleSheet.isEmpty())
			ui->BUTTON_OK->setStyleSheet(styleSheet);
		break;
	}

	case ButtonType_Cancel:
	{
		if (!styleSheet.isEmpty())
			ui->BUTTON_CANCEL->setStyleSheet(styleSheet);
		break;
	}

	default: break;
	}
}

void NotificationFrame::SetTimeout(unsigned int uiTimeout)
{
	if (uiTimeout > 10000)
		uiTimeout = 10000;
	m_uiTimeout = uiTimeout;
}

void NotificationFrame::SetMessageTitle(QString msgTitle)
{
	m_Title = msgTitle;
}

void NotificationFrame::SetMessageBody(QString msgBody)
{
	m_MsgBody = msgBody;
}

void NotificationFrame::Raise()
{
	ui->LABEL_MESSAGE->setText(QString("%1").arg(m_MsgBody));
	if (m_uiTimeout)
		m_Timer.start(m_uiTimeout);
	raise();
}

void NotificationFrame::Lower()
{
	m_Timer.stop();

	lower();
}

void NotificationFrame::on_BUTTON_OK_clicked()
{
	m_Timer.stop();

	lower();

	emit signalOk();
}

void NotificationFrame::on_BUTTON_CANCEL_clicked()
{
	m_Timer.stop();

	lower();

	emit signalCancel();
}

void NotificationFrame::slotTimer()
{
	m_Timer.stop();

	lower();

	emit signalCancel();
}
