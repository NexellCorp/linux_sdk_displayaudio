#ifndef NOTIFICATIONFRAME_H
#define NOTIFICATIONFRAME_H

#include <QFrame>
#include <NX_Type.h>

namespace Ui {
class NotificationFrame;
}

class NotificationFrame : public QFrame
{
	Q_OBJECT

public:
	explicit NotificationFrame(QWidget *parent = 0);
	~NotificationFrame();

	void Update();

private:
	Ui::NotificationFrame *ui;
};

#endif // NOTIFICATIONFRAME_H
