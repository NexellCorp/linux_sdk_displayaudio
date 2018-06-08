#include "PageIndicatorFrame.h"

PageIndicatorFrame::PageIndicatorFrame(QWidget *parent) :
	QFrame(parent)
{
	m_iPageCount = 0;
	m_iCurrentPage = -1;
}

PageIndicatorFrame::~PageIndicatorFrame()
{
}

void PageIndicatorFrame::setPageCount(int number)
{
	if (m_iPageCount != number) {
		m_iPageCount = number;
		update();
	}
}

void PageIndicatorFrame::setPage(int page)
{
	if (m_iCurrentPage != page) {
		m_iCurrentPage = page;
		update();
	}
}

void PageIndicatorFrame::paintEvent(QPaintEvent *)
{
	if (m_iPageCount == 0)
		return;

	QPainter painter(this);
	QBrush brush_active(QColor(48, 48, 50));
	QBrush brush_inactive(QColor(125, 125, 126));
	int y_center = height() / 2;
#if 0
	int interval = height();
	int rx = (float)height() / 3;
	int ry = (float)height() / 3;
#else
	int rx = (float)height() / 3;
	int ry = (float)height() / 3;
	int interval = rx * 3;
#endif
	int x_offset = width() / 2 - interval * (m_iPageCount / 2);
	painter.setPen(Qt::NoPen);
	for (int i = 0; i < m_iPageCount; ++i) {
		if (m_iCurrentPage == i) {
			painter.setBrush(brush_active);
		} else {
			painter.setBrush(brush_inactive);
		}
		painter.drawEllipse(QPoint(x_offset+(i*interval), y_center), rx, ry);
	}
}
