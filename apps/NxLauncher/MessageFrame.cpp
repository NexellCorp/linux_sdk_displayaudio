#include "MessageFrame.h"
#include "ui_MessageFrame.h"
#include "ShadowEffect.h"
#include <QDesktopWidget>

#define DEFAULT_WIDTH	1024
#define DEFAULT_HEIGHT	600

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

	m_uiTimeout = 0;

	connect(&m_Timer, SIGNAL(timeout()), this, SLOT(slotTimer()));

	const QRect screen = QApplication::desktop()->screenGeometry();
	if ((width() != screen.width()) || (height() != screen.height()))
	{
		setFixedSize(screen.width(), screen.height());
	}
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
	if (uiTimeout > 10000)
		uiTimeout = 10000;
	m_uiTimeout = uiTimeout;
}

void MessageFrame::SetMessageTitle(QString msgTitle)
{
	ui->LABEL_TITLE->setText(msgTitle);
}

void MessageFrame::SetMessageBody(QString msgBody)
{
	ui->LABEL_MESSAGE->setText(msgBody);
}

void MessageFrame::Raise()
{
	if (m_uiTimeout)
		m_Timer.start(m_uiTimeout);

	show();
	raise();
}

void MessageFrame::Lower()
{
	m_Timer.stop();

	hide();
	lower();
}

void MessageFrame::on_BUTTON_OK_clicked()
{
	m_Timer.stop();

	hide();
	lower();

	emit signalOk();
}

void MessageFrame::on_BUTTON_CANCEL_clicked()
{
	m_Timer.stop();

	hide();
	lower();

	emit signalCancel();
}

void MessageFrame::slotTimer()
{
	m_Timer.stop();

	hide();
	lower();

	emit signalCancel();
}

void MessageFrame::resizeEvent(QResizeEvent *)
{
	if ((width() != DEFAULT_WIDTH) || (height() != DEFAULT_HEIGHT))
	{
		SetupUI();
	}
}

void MessageFrame::SetupUI()
{
	int rx, ry;

	rx = width()/2 - ui->frame->width()/2;
	ry = height()/2 - ui->frame->height()/2;
	ui->frame->move(rx, ry);
}
