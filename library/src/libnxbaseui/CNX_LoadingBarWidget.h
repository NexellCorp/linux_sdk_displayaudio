#ifndef LOADINGBARWIDGET_H
#define LOADINGBARWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>

class CNX_LoadingBarWidget : public QWidget
{
	Q_OBJECT

private slots:
	void slotUpdate();

public:
	explicit CNX_LoadingBarWidget(QWidget *parent = 0);
	~CNX_LoadingBarWidget();

	void SetAngle(int iValue);
	int GetAngle() const;

	void SetThickness(int iValue);
	int GetThickness() const;

	void Start(int iPeriod); // millisecond
	void Stop();

protected:
	void paintEvent(QPaintEvent *event);

private:
	int m_iAngle;
	int m_iThickness;
	QTimer m_Timer;
};

#endif // LOADINGBARWIDGET_H
