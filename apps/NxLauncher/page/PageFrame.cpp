#include "PageFrame.h"
#include "ui_PageFrame.h"

PageFrame::PageFrame(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::PageFrame)
{
	ui->setupUi(this);
}

PageFrame::~PageFrame()
{
	delete ui;
}
