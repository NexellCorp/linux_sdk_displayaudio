#include "SelectMenuFrame.h"
#include "ui_SelectMenuFrame.h"

SelectMenuFrame::SelectMenuFrame(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::SelectMenuFrame)
{
	ui->setupUi(this);
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
