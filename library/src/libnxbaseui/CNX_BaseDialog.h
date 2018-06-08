#ifndef CNX_BASEDIALOG_H
#define CNX_BASEDIALOG_H

#include <QDialog>
#include <QFrame>
#include <QEvent>
#include <QPainter>

class CNX_BaseDialog : public QDialog
{
	Q_OBJECT

public:
	explicit CNX_BaseDialog(QWidget *parent = 0);
	~CNX_BaseDialog();

	void paintEvent(QPaintEvent *event);

protected:
	void SetWidget(QWidget* widget, int x = -1, int y = -1);

private:
	QWidget* m_pWidget;

	QRect m_ScreenGeometry;

	QImage m_Background;
};

#endif // CNX_BASEDIALOG_H
