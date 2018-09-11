#ifndef MESSAGEFRAME_H
#define MESSAGEFRAME_H

#include <QFrame>
#include <NX_Type.h>

namespace Ui {
class MessageFrame;
}

class MessageFrame : public QFrame
{
	Q_OBJECT

signals:
	void signalOk();

	void signalCancel();

public:
	explicit MessageFrame(QWidget *parent = 0);
	~MessageFrame();

	void SetRequestor(QString requestor);

	QString GetRequestor();

	void SetButtonVisibility(ButtonVisibility eVisibility);

	void SetButtonLocation(ButtonLocation eLocation);

	void SetButonStyleSheet(ButtonType eType, QString styleSheet);

	void SetTimeout(unsigned int uiTimeout);

	void SetMessageTitle(QString msgTitle);

	void SetMessageBody(QString msgBody);

//private slots:
//	void on_BUTTON_OK_clicked();

//	void on_BUTTON_CANCEL_clicked();

private slots:
	void on_BUTTON_OK_clicked();

	void on_BUTTON_CANCEL_clicked();

private:
	QString m_Requestor;

	bool m_bAccept;

private:
	Ui::MessageFrame *ui;
};

#endif // MESSAGEFRAME_H
