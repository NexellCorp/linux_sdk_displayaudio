#ifndef AVINDISPLAYVIEW_H
#define AVINDISPLAYVIEW_H

#include <QObject>
#include <QWidget>
#include <QGraphicsView>

class AVInDisplayView : public QGraphicsView
{
public:
	AVInDisplayView(QWidget *parent=NULL);
	void SetParentWindow( QWidget *parent );

protected:
	void mouseReleaseEvent(QMouseEvent *event);

private:
	QWidget *m_pParent;
};

#endif // AVINDISPLAYVIEW_H
