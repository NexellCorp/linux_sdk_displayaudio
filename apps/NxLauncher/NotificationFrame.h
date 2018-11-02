#ifndef NOTIFICATIONFRAME_H
#define NOTIFICATIONFRAME_H

#include <QFrame>
#include <QTimer>
#include <NX_Type.h>

namespace Ui {
class NotificationFrame;
}

class NotificationFrame : public QFrame
{
	Q_OBJECT

signals:
	void signalOk();

	void signalCancel();

private slots:
	void slotTimer();

public:
	explicit NotificationFrame(QWidget *parent = 0);
	~NotificationFrame();

	void SetRequestor(QString requestor);

	QString GetRequestor();

	void SetButtonVisibility(ButtonVisibility eVisibility);

	void SetButonStyleSheet(ButtonType eType, QString styleSheet);

	void SetTimeout(unsigned int uiTimeout);

	void SetMessageTitle(QString msgTitle);

	void SetMessageBody(QString msgBody);

	void Raise();

	void Lower();

private slots:
	void on_BUTTON_OK_clicked();

	void on_BUTTON_CANCEL_clicked();

private:
	QString m_Requestor;

	bool m_bAccept;

	unsigned int m_uiTimeout;

	QString m_Title;

	QString m_MsgBody;

	QTimer m_Timer;

private:
	Ui::NotificationFrame *ui;
};

#endif // NOTIFICATIONFRAME_H
