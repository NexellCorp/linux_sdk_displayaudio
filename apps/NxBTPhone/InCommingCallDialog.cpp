#include "InCommingCallDialog.h"
#include "ui_InCommingCallDialog.h"

InCommingCallDialog::InCommingCallDialog(QWidget *parent) :
    QDialog(parent, Qt::FramelessWindowHint),
    ui(new Ui::InCommingCallDialog)
{
    ui->setupUi(this);

	reset();
}

InCommingCallDialog::~InCommingCallDialog()
{
    delete ui;
}
