#include "ElideLabel.h"
#include <QResizeEvent>

ElideLabel::ElideLabel(QWidget *parent)
	: QLabel(parent)
{
	m_ElideMode = Qt::ElideLeft;
}

void ElideLabel::setText(const QString &s)
{
	m_Text = s;

	QLabel::setText(m_Text);
}

void ElideLabel::setText()
{
	QFontMetrics fm(font());
	QLabel::setText(fm.elidedText(m_Text, m_ElideMode, width()));
}

void ElideLabel::setElideMode(Qt::TextElideMode eMode)
{
	if (m_ElideMode != eMode)
	{
		m_ElideMode = eMode;
	}
}

void ElideLabel::resizeEvent(QResizeEvent *e)
{
	Q_UNUSED(e)

	setText();
}
