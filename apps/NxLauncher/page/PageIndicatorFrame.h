#ifndef PAGEINDICATORFRAME_H
#define PAGEINDICATORFRAME_H

#include <QFrame>
#include <QPainter>

class PageIndicatorFrame : public QFrame
{
	Q_OBJECT

public:
	explicit PageIndicatorFrame(QWidget *parent = 0);
	~PageIndicatorFrame();

	void setPage(int page);

	void setPageCount(int number);

	void paintEvent(QPaintEvent *);

private:
	int m_iCurrentPage;

	int m_iPageCount;
};

#endif // PAGEINDICATORFRAME_H
