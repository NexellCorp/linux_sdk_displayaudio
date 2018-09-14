#include "CNX_VolumeBar.h"
#include "ui_CNX_VolumeBar.h"

#define LOG_TAG "[CNX_Volume]"
#include <NX_Log.h>

#define MAX_COUNTDOWN 5

CNX_VolumeBar::CNX_VolumeBar(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::CNX_VolumeBar)
{
	ui->setupUi(this);

	ui->slider->installEventFilter(this);

	connect(&m_Timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
}

CNX_VolumeBar::~CNX_VolumeBar()
{
	delete ui;
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
	NXLOGI("[%s] val = %d", __FUNCTION__, ui->slider->value());

	lower();
}

void CNX_VolumeBar::raise()
{
	ResetCountDown();
	QFrame::raise();
}

void CNX_VolumeBar::slotTimer()
{
	if (--m_iCountDown == 0)
	{
		m_Timer.stop();
		lower();
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
