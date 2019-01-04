#include "TabSetting.h"
#include "ui_TabSetting.h"

TabSetting::TabSetting(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::TabSetting)
{
	ui->setupUi(this);
	ui->comboControlMethods->addItem( tr("ADB") );
	ui->comboControlMethods->addItem( tr("Network") );
	ui->comboControlMethods->addItem( tr("UART") );

	//	Set to ADB Mode by default
	m_CurUIMode = UI_MOD_ADB;
	ui->comboControlMethods->setCurrentIndex(0);
	UpdateUIMode( m_CurUIMode );

	UpdateSetting();
}

TabSetting::~TabSetting()
{
	delete ui;
}

void TabSetting::UpdateSetting()
{
	//	Update Setting Informations
	static BTN_SETTINGS setting;
	memset( &setting, 0, sizeof(setting) );
	strncpy( setting.adbCommand, ui->lineEdtAdbConCmd->text().toStdString().c_str(), sizeof(setting.adbCommand) );
	strncpy( setting.ipAddress, ui->lineEdtIPAddress->text().toStdString().c_str(), sizeof(setting.ipAddress) );
	setting.portNumber = ui->lineEditPortNumber->text().toShort();
	strncpy( setting.uartPortString, ui->lineEdtUartPort->text().toStdString().c_str(), sizeof(setting.uartPortString) );
	SetSettings(&setting);
}

void TabSetting::UpdateUIMode( uint32_t uiMode )
{
	if( UI_MOD_ADB == uiMode )
	{
		UpdateUIADB(true);
		UpdateUINetwork(false);
		UpdateUIUart(false);
	}
	else if( UI_MOD_NETWORK == uiMode )
	{
		UpdateUIADB(false);
		UpdateUINetwork(true);
		UpdateUIUart(false);
	}
	else if( UI_MOD_UART == uiMode )
	{
		UpdateUIADB(false);
		UpdateUINetwork(false);
		UpdateUIUart(true);
	}
}

void TabSetting::UpdateUIADB( bool enable )
{
	if( enable )
	{
		ui->labelADBCommand->show();
		ui->lineEdtAdbConCmd->show();
	}
	else
	{
		ui->labelADBCommand->hide();
		ui->lineEdtAdbConCmd->hide();
	}
}

void TabSetting::UpdateUINetwork( bool enable )
{
	if( enable )
	{
		ui->labelNetworkSetting->show();
		ui->labelIPAddress->show();
		ui->labelPortNumber->show();
		ui->lineEdtIPAddress->show();
		ui->lineEditPortNumber->show();
	}
	else
	{
		ui->labelNetworkSetting->hide();
		ui->labelIPAddress->hide();
		ui->labelPortNumber->hide();
		ui->lineEdtIPAddress->hide();
		ui->lineEditPortNumber->hide();
	}
}

void TabSetting::UpdateUIUart( bool enable )
{
	if( enable )
	{
		ui->labelUartPort->show();
		ui->lineEdtUartPort->show();
	}
	else
	{
		ui->labelUartPort->hide();
		ui->lineEdtUartPort->hide();
	}
}

void TabSetting::on_comboControlMethods_currentIndexChanged(const QString &arg1)
{
	if( tr("ADB") == arg1 )
	{
		m_CurUIMode = UI_MOD_ADB;
	}
	else if( tr("Network") == arg1 )
	{
		m_CurUIMode = UI_MOD_NETWORK;
	}
	else if( tr("UART") == arg1 )
	{
		m_CurUIMode = UI_MOD_UART;
	}
	UpdateUIMode( m_CurUIMode );
}



void TabSetting::on_lineEdtAdbConCmd_textChanged(const QString &arg1)
{
	(void)arg1;
	UpdateSetting();
}

void TabSetting::on_lineEdtIPAddress_textChanged(const QString &arg1)
{
	(void)arg1;
	UpdateSetting();
}

void TabSetting::on_lineEditPortNumber_textChanged(const QString &arg1)
{
	(void)arg1;
	UpdateSetting();
}

void TabSetting::on_lineEdtUartPort_textChanged(const QString &arg1)
{
	(void)arg1;
	UpdateSetting();
}
