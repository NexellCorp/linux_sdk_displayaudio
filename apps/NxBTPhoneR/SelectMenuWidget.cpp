#include "SelectMenuWidget.h"
#include "ui_SelectMenuWidget.h"

SelectMenuWidget::SelectMenuWidget(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint),
    ui(new Ui::SelectMenuWidget)
{
    ui->setupUi(this);
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
