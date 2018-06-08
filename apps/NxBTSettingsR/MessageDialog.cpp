#include "MessageDialog.h"
#include "ui_MessageDialog.h"
#include <QDesktopWidget>

MessageDialog::MessageDialog(QWidget *parent) :
    QDialog(parent, Qt::FramelessWindowHint),
    ui(new Ui::MessageDialog)
{
    ui->setupUi(this);

    // move to center position
    const QRect r = QApplication::desktop()->screenGeometry();
    move(r.width() / 2 - width() / 2, r.height() / 2 - height() / 2);

    m_ButtonStatePolicy = ButtonStatePolicy_Ok_And_Cancel;
}

MessageDialog::~MessageDialog()
{
    delete ui;
}

void MessageDialog::setTitle(QString text)
{
    ui->LABEL_TITLE->setText(text);
}

void MessageDialog::setMessage(QString text)
{
    ui->LABEL_MESSAGE->setText(text);
}

void MessageDialog::on_BUTTON_CANCEL_clicked()
{
    reject();
}

void MessageDialog::on_BUTTON_OK_clicked()
{
    accept();
}

void MessageDialog::setButtonStatePolicy(ButtonStatePolicy policy)
{
    if (m_ButtonStatePolicy == policy)
        return;

    switch (policy) {
    case ButtonStatePolicy_Ok_And_Cancel:
        ui->BUTTON_OK->show();
        ui->BUTTON_CANCEL->show();
        break;

    case ButtonStatePolicy_Only_Ok:
        ui->BUTTON_OK->show();
        ui->BUTTON_CANCEL->hide();

        // move center position
        ui->BUTTON_OK->move(width()/2 - ui->BUTTON_OK->width()/2, ui->BUTTON_OK->y());
        break;

    case ButtonStatePolicy_Only_Cancel:
        ui->BUTTON_OK->hide();
        ui->BUTTON_CANCEL->show();

        // move center position
        ui->BUTTON_CANCEL->move(width()/2 - ui->BUTTON_CANCEL->width()/2, ui->BUTTON_CANCEL->y());
        break;
    }
}

void MessageDialog::setButtonText(ButtonType type, QString text)
{
    switch (type) {
    case ButtonType_Ok:
        ui->BUTTON_OK->setText(text);
        break;

    case ButtonType_Cancel:
        ui->BUTTON_CANCEL->setText(text);
        break;
    }
}
