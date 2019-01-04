#include "CarBtnWindow.h"
#include "ui_CarBtnWindow.h"

CarBtnWindow::CarBtnWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::CarBtnWindow)
{
	ui->setupUi(this);

	this->resize( 800,600 );
	m_TabWidget = new QTabWidget(this);
	m_TabWidget->resize( 800, 600 );
	m_TabWidget->addTab( new TabButtons(), tr("Button") );
	m_TabWidget->addTab( new TabSetting(), tr("Setting") );
	m_TabWidget->addTab( new TabTest(), tr("Test") );
}

CarBtnWindow::~CarBtnWindow()
{
	delete ui;
}
