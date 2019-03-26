#include "CNX_LoadingBarWidget.h"

CNX_LoadingBarWidget::CNX_LoadingBarWidget(QWidget *parent) :
	QWidget(parent)
{
	connect(&m_Timer, SIGNAL(timeout()), this, SLOT(slotUpdate()));

	SetThickness(10);
	SetAngle(0);
}

CNX_LoadingBarWidget::~CNX_LoadingBarWidget()
{
	m_Timer.stop();
}

void CNX_LoadingBarWidget::SetAngle(int iValue)
{
	m_iAngle = iValue;
}

int CNX_LoadingBarWidget::GetAngle() const
{
	return m_iAngle;
}

void CNX_LoadingBarWidget::SetThickness(int iValue)
{
	m_iThickness = iValue;
}

int CNX_LoadingBarWidget::GetThickness() const
{
	return m_iThickness;
}

void CNX_LoadingBarWidget::paintEvent(QPaintEvent *)
{
	QRect drawingRect;
	drawingRect.setX(rect().x() + m_iThickness);
	drawingRect.setY(rect().y() + m_iThickness);
	drawingRect.setWidth(rect().width() - m_iThickness * 2);
	drawingRect.setHeight(rect().height() - m_iThickness * 2);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	QConicalGradient gradient;
	gradient.setCenter(drawingRect.center());
	gradient.setAngle(m_iAngle);
	gradient.setColorAt(0, QColor(0, 0, 0));
	gradient.setColorAt(1, QColor(255, 255, 255));

	QPen pen(QBrush(gradient), m_iThickness);
	pen.setCapStyle(Qt::RoundCap);
	painter.setPen(pen);
	painter.drawArc(drawingRect, 0 * 16, 360 * 16);
}

void CNX_LoadingBarWidget::slotUpdate()
{
	m_iAngle += 16;
	if (m_iAngle > 360)
		SetAngle(0);
	update();
}

void CNX_LoadingBarWidget::Start(int iPeriod)
{
	if (iPeriod < 0)
	{
		iPeriod = 0;
	}

	if (m_Timer.isActive())
	{
		m_Timer.stop();
	}

	m_Timer.start(iPeriod);
}

void CNX_LoadingBarWidget::Stop()
{
	m_Timer.stop();
}
