#include "NotificationFrame.h"
#include "ui_NotificationFrame.h"

NotificationFrame::NotificationFrame(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::NotificationFrame)
{
	ui->setupUi(this);
}

NotificationFrame::~NotificationFrame()
{
	delete ui;
}
