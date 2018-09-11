#ifndef KEYBOARDFRAME_H
#define KEYBOARDFRAME_H

#include <QFrame>

namespace Ui {
class KeyboardFrame;
}

class KeyboardFrame : public QFrame
{
	Q_OBJECT

signals:
	void signalAccepted();

	void signalRejected();

public:
	explicit KeyboardFrame(QWidget *parent = 0);
	~KeyboardFrame();

protected:
	void keyPressEvent(QKeyEvent *);

	void mousePressEvent(QMouseEvent *event);

	void mouseMoveEvent(QMouseEvent *event);

	bool eventFilter(QObject *watched, QEvent *event);

public:
	void setText(QString text);

	QString text();

private slots:
	void on_BUTTON_CLOSE_clicked();

private:
	void initialize();

private:
	bool m_bMoveable;
	QPoint m_StartPt;

private:
	Ui::KeyboardFrame *ui;
};

#endif // KEYBOARDFRAME_H
