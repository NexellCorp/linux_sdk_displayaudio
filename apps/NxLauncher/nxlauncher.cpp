#include "nxlauncher.h"
#include "ui_nxlauncher.h"



#define NX_LAUNCHER "NxLauncher"


NxLauncher::NxLauncher(QWidget *parent) :
	QMainWindow(parent, Qt::FramelessWindowHint),
	ui(new Ui::NxLauncher)
{
	ui->setupUi(this);
}

NxLauncher::~NxLauncher()
{
	removeEventFilter(this);

	delete ui;
}

