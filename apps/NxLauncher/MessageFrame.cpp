#include "MessageFrame.h"
#include "ui_MessageFrame.h"
#include "ShadowEffect.h"

MessageFrame::MessageFrame(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::MessageFrame)
{
	ui->setupUi(this);

	ShadowEffect *bodyShadow = new ShadowEffect(this);
	bodyShadow->setBlurRadius(55.0);
	bodyShadow->setDistance(10);
	bodyShadow->setColor(QColor(0, 0, 0, 100));

	ui->frame->setGraphicsEffect(bodyShadow);
}

MessageFrame::~MessageFrame()
{
	delete ui;
}

void MessageFrame::SetRequestor(QString requestor)
{
	m_Requestor = requestor;
}

QString MessageFrame::GetRequestor()
{
	return m_Requestor;
}

void MessageFrame::SetButtonVisibility(ButtonVisibility eVisibility)
{
	switch (eVisibility) {
	case ButtonVisibility_Ok:
		ui->BUTTON_OK->show();
		ui->BUTTON_CANCEL->hide();
		ui->BUTTON_OK->move(ui->frame->width()/2-ui->BUTTON_OK->width()/2, ui->BUTTON_OK->y());
		break;

	case ButtonVisibility_Cencel:
		ui->BUTTON_CANCEL->show();
		ui->BUTTON_OK->hide();
		ui->BUTTON_CANCEL->move(ui->frame->width()/2-ui->BUTTON_CANCEL->width()/2, ui->BUTTON_CANCEL->y());
		break;

	case ButtonVisibility_Default:
		ui->BUTTON_OK->show();
		ui->BUTTON_CANCEL->show();
		ui->BUTTON_CANCEL->move(160, ui->BUTTON_CANCEL->y());
		ui->BUTTON_OK->move(440, ui->BUTTON_OK->y());
		break;

	default: break;
	}
}

void MessageFrame::SetButtonLocation(ButtonLocation eLocation)
{

}

void MessageFrame::SetButonStyleSheet(ButtonType eType, QString styleSheet)
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

void MessageFrame::SetTimeout(unsigned int uiTimeout)
{

}

void MessageFrame::SetMessageTitle(QString msgTitle)
{
	ui->LABEL_TITLE->setText(msgTitle);
}

void MessageFrame::SetMessageBody(QString msgBody)
{
	ui->LABEL_MESSAGE->setText(msgBody);
}

void MessageFrame::on_BUTTON_OK_clicked()
{
	lower();

	emit signalOk();
}

void MessageFrame::on_BUTTON_CANCEL_clicked()
{
	lower();

	emit signalCancel();
}
