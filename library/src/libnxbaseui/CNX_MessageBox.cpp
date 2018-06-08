#include "CNX_MessageBox.h"
#include "ui_CNX_MessageBox.h"

#define SCREEN_WIDTH        1024
#define SCREEN_HEIGHT       600

CNX_MessageBox::CNX_MessageBox(QWidget *parent) :
    QDialog(parent),
	ui(new Ui::CNX_MessageBox)
{
    ui->setupUi(this);

    setFixedSize( SCREEN_WIDTH, SCREEN_HEIGHT );
//    setStyleSheet("background:transparent;");
//    setAttribute(Qt::WA_TranslucentBackground);
//    setWindowFlags(Qt::FramelessWindowHint);
}

CNX_MessageBox::~CNX_MessageBox()
{
    delete ui;
}

void CNX_MessageBox::show()
{
    QDialog::show();
    m_MsgBox.show();
}

void CNX_MessageBox::hide()
{
    QDialog::hide();
    m_MsgBox.hide();
}

int CNX_MessageBox::exec()
{
	return QDialog::exec();
//    m_MsgBox.exec();
}

void CNX_MessageBox::setDefaultButton(QPushButton * button)
{
    m_MsgBox.setDefaultButton(button);
}

void CNX_MessageBox::setDetailedText(const QString & text)
{
    m_MsgBox.setDetailedText(text);
}

void CNX_MessageBox::setEscapeButton(QAbstractButton * button)
{
    m_MsgBox.setEscapeButton(button);
}

void CNX_MessageBox::setInformativeText(const QString & text)
{
    m_MsgBox.setInformativeText(text);
}

void CNX_MessageBox::setText(const QString & text)
{
    m_MsgBox.setText(text);
}

void CNX_MessageBox::setTextFormat(Qt::TextFormat format)
{
    m_MsgBox.setTextFormat(format);
}

void CNX_MessageBox::setWindowModality(Qt::WindowModality windowModality)
{
    m_MsgBox.setWindowModality(windowModality);
}

void CNX_MessageBox::setWindowTitle(const QString & title)
{
    m_MsgBox.setWindowTitle(title);
}
