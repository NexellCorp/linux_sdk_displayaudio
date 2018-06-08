#include <unistd.h>
#include "TabTest.h"
#include "ui_TabTest.h"
#include "DAudioKeyDef.h"
#include "CSettings.h"

TabTest::TabTest(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::TabTest)
{
	ui->setupUi(this);

	m_bStartedAudFocusTest = false;
	m_bExitAudioFocusTestLoop = true;

    m_bTestDualDisplay = false;
}

TabTest::~TabTest()
{
	delete ui;
}

void TabTest::on_btnAudioFocusTest_clicked()
{
	if( m_bStartedAudFocusTest )
	{
		m_bExitAudioFocusTestLoop = true;
		pthread_join(m_hThreadAudFocus, NULL);
		m_bStartedAudFocusTest = false;
	}
	else
	{
		m_bExitAudioFocusTestLoop = false;
		pthread_create( &m_hThreadAudFocus, NULL, ThreadStubAudFocus, this );
		m_bStartedAudFocusTest = true;
	}
}

static int32_t AdbSend( const char *pCmd, int32_t key, int32_t value )
{
    char cmdStr[128];
    sprintf( cmdStr, "%s -k %d -v %d", pCmd, key, value );
    qDebug("%s\n", cmdStr);
    system( cmdStr );
    return 0;
}

void TabTest::on_btnAVMStart_clicked()
{
    BTN_SETTINGS *pSetting;
    GetSettings(&pSetting);

//    if( !m_bTestDualDisplay )
    {
        //AdbSend(pSetting->adbCommand, DAUDIO_KEY::DAUD_KEY_MODE_3DAVM , KEY_RELEASED);
        system("adb shell NxCommandSender 1");
        m_bTestDualDisplay = true;
    }
}

void TabTest::on_btnAVMStop_clicked()
{
    BTN_SETTINGS *pSetting;
    GetSettings(&pSetting);

//    if( m_bTestDualDisplay )
    {
        //AdbSend(pSetting->adbCommand, DAUDIO_KEY::DAUD_KEY_MODE_3DAVM_CLOSE , KEY_RELEASED);
        system("adb shell NxCommandSender 0");
        m_bTestDualDisplay = false;
    }
}

void TabTest::ThreadProcAudFocus()
{
	BTN_SETTINGS *pSetting;
	GetSettings(&pSetting);

	while( !m_bExitAudioFocusTestLoop )
	{
		AdbSend(pSetting->adbCommand, DAUDIO_KEY::DAUD_KEY_MODE_AUDIO , KEY_RELEASED);
		if( m_bExitAudioFocusTestLoop )
			break;
		usleep( 2000000 );
		AdbSend(pSetting->adbCommand, DAUDIO_KEY::DAUD_KEY_MODE_VIDEO , KEY_RELEASED);
		if( m_bExitAudioFocusTestLoop )
			break;
		usleep( 3000000 );
	}
}
