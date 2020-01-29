#include "CNX_VolumeBar.h"
#include "ui_CNX_VolumeBar.h"

#include "ShadowEffect.h"

#define LOG_TAG "[CNX_Volume]"
#include <NX_Log.h>

#define MAX_COUNTDOWN 5

CNX_VolumeBar::CNX_VolumeBar(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::CNX_VolumeBar)
{
	ui->setupUi(this);

	ShadowEffect *bodyShadow = new ShadowEffect(this);
	bodyShadow->setBlurRadius(55.0);
	bodyShadow->setDistance(10);
	bodyShadow->setColor(QColor(180, 180, 180, 200));

	ui->slider->setGraphicsEffect(bodyShadow);

	ui->slider->installEventFilter(this);

	connect(&m_Timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
}

CNX_VolumeBar::~CNX_VolumeBar()
{
	delete ui;
}
#include <QDebug>
bool CNX_VolumeBar::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == ui->slider)
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent *e = static_cast<QMouseEvent *>(event);
			ui->slider->setValue(ui->slider->maximum() * e->pos().x() / ui->slider->width());
			ResetCountDown();
		}
	}

	return QFrame::eventFilter(watched, event);
}

void CNX_VolumeBar::on_slider_sliderMoved(int position)
{
	(void)position;
	ResetCountDown();
}

void CNX_VolumeBar::on_slider_sliderPressed()
{
	ResetCountDown();
}

void CNX_VolumeBar::on_slider_sliderReleased()
{
	lower();
	hide();

	emit signalSetVolume(ui->slider->value());
}

void CNX_VolumeBar::Raise()
{
	ResetCountDown();
	if (isHidden())
	{
		show();
	}
	raise();
}

void CNX_VolumeBar::slotTimer()
{
	if (--m_iCountDown == 0)
	{
		m_Timer.stop();
		lower();
		hide();
		emit signalSetVolume(ui->slider->value());
		return;
	}

	ui->slider->setWindowOpacity((qreal)m_iCountDown/MAX_COUNTDOWN);
}

void CNX_VolumeBar::ResetCountDown()
{
	m_iCountDown = MAX_COUNTDOWN;
	m_Timer.stop();
	m_Timer.start(1000);
}

void CNX_VolumeBar::SetValue(int value)
{
	ui->slider->setValue(value);
}
