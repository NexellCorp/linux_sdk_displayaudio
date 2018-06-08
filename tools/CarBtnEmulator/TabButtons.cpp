#include "TabButtons.h"
#include "ui_TabButtons.h"
#include "DAudioKeyDef.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

TabButtons::TabButtons(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::TabButtons)
{
	ui->setupUi(this);
	//
	//	Set Default Send Method to ADB
	//
	ui->radioADB->setChecked(true);
	ui->radioUART->setDisabled(true);
}

TabButtons::~TabButtons()
{
	delete ui;
}


static int32_t RemoteSend( const char *addr, uint16_t port, int32_t key, int32_t value )
{
	int32_t ret = 0;
	int32_t data[2];
	int32_t clntSock;
	clntSock  = socket( AF_INET, SOCK_DGRAM, 0);
	if( -1 == clntSock)
	{
		return -1;
	}

	data[0] = key;
	data[1] = value;

	struct sockaddr_in destAddr;
	memset( &destAddr, 0, sizeof(destAddr) );
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(port);
	destAddr.sin_addr.s_addr = inet_addr(addr);

	socklen_t addrLen = sizeof(destAddr);

	ssize_t sendSize = sendto(clntSock , data, sizeof(data), 0, (struct sockaddr*)&destAddr, addrLen);

	if( sendSize < 0 )
	{
		printf("send failed!!!\n");
		ret = -1;
	}

	close(clntSock);
	return ret;
}

static int32_t AdbSend( const char *pCmd, int32_t key, int32_t value )
{
	char cmdStr[128];
	sprintf( cmdStr, "%s -k %d -v %d", pCmd, key, value );
	qDebug("%s\n", cmdStr);
	system( cmdStr );
	return 0;
}

bool TabButtons::SendKey( int32_t key, int32_t value )
{
	BTN_SETTINGS *pSetting = NULL;
	GetSettings( &pSetting );
	if( ui->radioADB->isChecked() )
	{
		AdbSend( pSetting->adbCommand, key, value );
	}
	else if( ui->radioNetwork->isChecked() )
	{
		RemoteSend(pSetting->ipAddress, pSetting->portNumber, key, value);
	}
	else
	{
	}
	return true;
}

//
//	Power Control
//
void TabButtons::on_btnPower_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_POWER, KEY_RELEASED );
}
void TabButtons::on_btnPower_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_POWER, KEY_PRESSED);
}


//
//	Mode
//
void TabButtons::on_btnMode_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE, KEY_RELEASED);
}
void TabButtons::on_btnVideo_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE_VIDEO, KEY_RELEASED);
}
void TabButtons::on_btnAudio_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE_AUDIO, KEY_RELEASED);
}
void TabButtons::on_btnRadio_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE_RADIO, KEY_RELEASED);
}
void TabButtons::on_btnAVIn_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE_AVIN, KEY_RELEASED);
}
void TabButtons::on_btnBluetooth_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE_BLUETOOTH, KEY_RELEASED);
}
void TabButtons::on_btnPhone_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE_PHONE, KEY_RELEASED);
}
void TabButtons::on_btnSettings_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE_SETTING, KEY_RELEASED);
}


void TabButtons::on_btnMode_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE, KEY_PRESSED);
}
void TabButtons::on_btnVideo_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE_VIDEO, KEY_PRESSED);
}
void TabButtons::on_btnAudio_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE_AUDIO, KEY_PRESSED);
}
void TabButtons::on_btnRadio_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE_RADIO, KEY_PRESSED);
}
void TabButtons::on_btnAVIn_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE_AVIN, KEY_PRESSED);
}
void TabButtons::on_btnBluetooth_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE_BLUETOOTH, KEY_PRESSED);
}
void TabButtons::on_btnPhone_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE_PHONE, KEY_PRESSED);
}
void TabButtons::on_btnSettings_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_MODE_SETTING, KEY_PRESSED);
}




//
//	Volume Up/Down Control
//
void TabButtons::on_btnVolUp_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_VOL_UP, KEY_RELEASED);
}
void TabButtons::on_btnVolDown_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_VOL_DOWN, KEY_RELEASED);
}
void TabButtons::on_btnVolMute_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_VOL_MUTE, KEY_RELEASED);
}

void TabButtons::on_btnVolUp_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_VOL_UP, KEY_PRESSED);
}
void TabButtons::on_btnVolDown_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_VOL_DOWN, KEY_PRESSED);
}
void TabButtons::on_btnVolMute_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_VOL_MUTE, KEY_PRESSED);
}


//
//	Navigation Up/Down/Left/Right
//
void TabButtons::on_btnUp_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_NAVI_UP, KEY_RELEASED);
}

void TabButtons::on_btnDown_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_NAVI_DOWN, KEY_RELEASED);
}

void TabButtons::on_btnLeft_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_NAVI_LEFT, KEY_RELEASED);
}

void TabButtons::on_btnRight_released()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_NAVI_RIGHT, KEY_RELEASED);
}

void TabButtons::on_btnUp_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_NAVI_UP, KEY_PRESSED);
}
void TabButtons::on_btnDown_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_NAVI_DOWN, KEY_PRESSED);
}
void TabButtons::on_btnLeft_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_NAVI_LEFT, KEY_PRESSED);
}
void TabButtons::on_btnRight_pressed()
{
	SendKey( DAUDIO_KEY::DAUD_KEY_NAVI_RIGHT, KEY_PRESSED);
}

