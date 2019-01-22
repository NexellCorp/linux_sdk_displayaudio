#include "SelectMenuWidget.h"
#include "ui_SelectMenuWidget.h"
#include <QDesktopWidget>

#define DEFAULT_WIDTH	1024
#define DEFAULT_HEIGHT	540

SelectMenuWidget::SelectMenuWidget(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint),
    ui(new Ui::SelectMenuWidget)
{
    ui->setupUi(this);

	const QRect screen = QApplication::desktop()->screenGeometry();
	if ((width() != screen.width()) || (height() != screen.height()))
	{
		setFixedSize(screen.width(), screen.height() * 0.9);
	}
}

SelectMenuWidget::~SelectMenuWidget()
{
    delete ui;
}

void SelectMenuWidget::on_BUTTON_ENTER_CALL_MENU_clicked()
{
    emit signalCurrentMenuChanged(Menu_Call);
}

void SelectMenuWidget::on_BUTTON_ENTER_MESSAGE_MENU_clicked()
{
    emit signalCurrentMenuChanged(Menu_Message);
}

void SelectMenuWidget::resizeEvent(QResizeEvent *)
{
	if ((width() != DEFAULT_WIDTH) || (height() != DEFAULT_HEIGHT))
	{
		SetupUI();
	}
}

void SelectMenuWidget::SetupUI()
{
	float widthRatio = (float)width() / DEFAULT_WIDTH;
	float heightRatio = (float)height() / DEFAULT_HEIGHT;
	int rx, ry, rw, rh;

	rx = widthRatio * ui->BUTTON_ENTER_CALL_MENU->x();
	ry = heightRatio * ui->BUTTON_ENTER_CALL_MENU->y();
	rw = widthRatio * ui->BUTTON_ENTER_CALL_MENU->width();
	rh = heightRatio * ui->BUTTON_ENTER_CALL_MENU->height();
	ui->BUTTON_ENTER_CALL_MENU->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->BUTTON_ENTER_MESSAGE_MENU->x();
	ry = heightRatio * ui->BUTTON_ENTER_MESSAGE_MENU->y();
	rw = widthRatio * ui->BUTTON_ENTER_MESSAGE_MENU->width();
	rh = heightRatio * ui->BUTTON_ENTER_MESSAGE_MENU->height();
	ui->BUTTON_ENTER_MESSAGE_MENU->setGeometry(rx, ry, rw, rh);
}
