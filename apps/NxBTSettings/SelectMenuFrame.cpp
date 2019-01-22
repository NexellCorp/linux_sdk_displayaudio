#include "SelectMenuFrame.h"
#include "ui_SelectMenuFrame.h"
#include <QDesktopWidget>

#define DEFAULT_WIDTH	1024
#define DEFAULT_HEIGHT	540

SelectMenuFrame::SelectMenuFrame(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::SelectMenuFrame)
{
	ui->setupUi(this);

	const QRect screen = QApplication::desktop()->screenGeometry();
	if ((width() != screen.width()) || (height() != screen.height()))
	{
		setFixedSize(screen.width(), screen.height() * 0.9);
	}
}

SelectMenuFrame::~SelectMenuFrame()
{
	delete ui;
}

void SelectMenuFrame::on_BUTTON_ENTER_CONNECTION_MENU_clicked()
{
	emit signalCurrentMenuChanged(Menu_Connection);
}

void SelectMenuFrame::on_BUTTON_ENTER_ADVANCED_MENU_clicked()
{
	emit signalCurrentMenuChanged(Menu_Advanced);
}

void SelectMenuFrame::resizeEvent(QResizeEvent *)
{
	if ((width() != DEFAULT_WIDTH) || (height() != DEFAULT_HEIGHT))
	{
		SetupUI();
	}
}

void SelectMenuFrame::SetupUI()
{
	float widthRatio = (float)width() / DEFAULT_WIDTH;
	float heightRatio = (float)height() / DEFAULT_HEIGHT;
	int rx, ry, rw, rh;

	rx = widthRatio * ui->BUTTON_ENTER_CONNECTION_MENU->x();
	ry = heightRatio * ui->BUTTON_ENTER_CONNECTION_MENU->y();
	rw = widthRatio * ui->BUTTON_ENTER_CONNECTION_MENU->width();
	rh = heightRatio * ui->BUTTON_ENTER_CONNECTION_MENU->height();
	ui->BUTTON_ENTER_CONNECTION_MENU->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->BUTTON_ENTER_ADVANCED_MENU->x();
	ry = heightRatio * ui->BUTTON_ENTER_ADVANCED_MENU->y();
	rw = widthRatio * ui->BUTTON_ENTER_ADVANCED_MENU->width();
	rh = heightRatio * ui->BUTTON_ENTER_ADVANCED_MENU->height();
	ui->BUTTON_ENTER_ADVANCED_MENU->setGeometry(rx, ry, rw, rh);
}
