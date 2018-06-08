#include "avinmainwindow.h"
#include "ui_avinmainwindow.h"

#include <NX_AVIn.h>
#include <nx-v4l2.h>
#include <drm/drm_fourcc.h>
#include <unistd.h>

#include <NX_PacpClient.h>

static void cbStatusHome( void *pObj );
static void cbStatusBack( void *pObj );

AVInMainWindow::AVInMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::AVInMainWindow),
	m_bShowAVIn( false )
{
	ui->setupUi(this);

	NX_PacpClientStart(this);
	installEventFilter( this );

	m_pStatusBar = new CNX_StatusBar( this );
	m_pStatusBar->move( 0, 0 );
	m_pStatusBar->resize( this->size().width(), this->size().height() * 1 / 10 );
	m_pStatusBar->RegOnClickedHome( cbStatusHome );
	m_pStatusBar->RegOnClickedBack( cbStatusBack );
	m_pStatusBar->SetTitleName( "Nexell AVIN" );

	m_pStatusBar->show();
	m_bShowStatusBar = true;

	ui->graphicsView->SetParentWindow( this );
	ui->graphicsView->show();
}

AVInMainWindow::~AVInMainWindow()
{
	StopAVIn();
	removeEventFilter( this );
	NX_PacpClientStop();
	delete ui;
}

bool AVInMainWindow::ShowAVIn()
{
	if( m_bShowAVIn )
		return false;

	ui->graphicsView->show();

	int32_t planeId   = 27;	//	DRM plain ID
	int32_t crtcId    = 26;	//	DRM crtc ID
	int32_t camWidth  = 704;
	int32_t camHeight = 480;

	memset( &m_CamInfo, 0, sizeof(m_CamInfo) );
	memset( &m_DspInfo, 0, sizeof(m_DspInfo) );

	//  Camera Information
	m_CamInfo.iModule		= 1;
	m_CamInfo.bInterlace	= 1;
	m_CamInfo.iSensorId		= nx_sensor_subdev;
	m_CamInfo.iWidth		= camWidth;
	m_CamInfo.iHeight		= camHeight;
	m_CamInfo.iCropX		= 0;
	m_CamInfo.iCropY		= 0;
	m_CamInfo.iCropWidth	= camWidth;
	m_CamInfo.iCropHeight	= camHeight;
	m_CamInfo.iOutWidth     = camWidth;
	m_CamInfo.iOutHeight	= camHeight;

	//	Get graphic view's rect
	m_DspInfo.iPlaneId		= planeId;
	m_DspInfo.iCrtcId		= crtcId;
	m_DspInfo.uDrmFormat	= DRM_FORMAT_YUV420;
	m_DspInfo.iSrcWidth		= camWidth;
	m_DspInfo.iSrcHeight	= camHeight;
	m_DspInfo.iCropX		= 0;
	m_DspInfo.iCropY		= 0;
	m_DspInfo.iCropWidth	= camWidth;
	m_DspInfo.iCropHeight	= camHeight;
	m_DspInfo.iDspX			= ui->graphicsView->geometry().x();
	m_DspInfo.iDspY			= ui->graphicsView->geometry().y();
	m_DspInfo.iDspWidth 	= ui->graphicsView->geometry().width();
	m_DspInfo.iDspHeight	= ui->graphicsView->geometry().height();

	if( 0 != NXDA_StartAVInService( &m_CamInfo, &m_DspInfo ) )
	{
		printf("Fail, NXDA_StartAVInService().\n");
		return false;
	}

	m_bShowAVIn = true;
	return true;
}

void AVInMainWindow::StopAVIn()
{
	if( !m_bShowAVIn )
		return ;

	NXDA_StopAVInService();
	ui->graphicsView->hide();

	m_bShowAVIn = false;
}

bool AVInMainWindow::IsShowAVIn()
{
	return m_bShowAVIn;
}

void AVInMainWindow::ToggleStatusBar()
{
	if( m_bShowStatusBar )
	{
		m_pStatusBar->hide();
		m_bShowStatusBar = false;
	}
	else
	{
		m_pStatusBar->show();
		m_bShowStatusBar = true;
	}
}

//
//  Dialog Event Override.
//
bool AVInMainWindow::eventFilter(QObject *watched, QEvent *event)
{
	if( event->type() == NX_QT_CUSTOM_EVENT_TYPE )
	{
		NxEvent *pEvent = reinterpret_cast<NxEvent*>(event);

		switch( pEvent->m_iEventType )
		{
		case NX_REQUEST_PROCESS_SHOW:
			NX_PacpClientRequestRaise();
			break;
		}
	}

	if( (event->type() == QEvent::ActivationChange) && isActiveWindow() )
	{
		printf(">>>>> NX_ReplyDone(). ( %d )\n", NX_ReplyDone() );
	}

	return QMainWindow::eventFilter(watched, event);
}

void AVInMainWindow::showEvent( QShowEvent* event )
{
	Q_UNUSED( event );

	if( !isHidden() )
		return ;

	int32_t iRet = NX_RequestCommand( NX_REQUEST_PROCESS_SHOW );
	if( NX_REPLY_DONE != iRet )
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_PROCESS_SHOW, iRet );
	}

	QMainWindow::show();
}

void AVInMainWindow::hideEvent( QHideEvent* event )
{
	Q_UNUSED( event );

	if( isHidden() )
		return ;

	int32_t iRet = NX_RequestCommand( NX_REQUEST_PROCESS_HIDE );
	if( NX_REPLY_DONE != iRet )
	{
		printf("Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_PROCESS_HIDE, iRet );
	}

	QMainWindow::hide();
}

void AVInMainWindow::closeEvent( QCloseEvent* event )
{
	Q_UNUSED( event );

	StopAVIn();

	int32_t iRet;
	iRet = NX_RequestCommand( NX_REQUEST_FOCUS_VIDEO_LOSS );
	if( NX_REPLY_DONE != iRet )
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_FOCUS_VIDEO_LOSS, iRet );
		return ;
	}

#if ENABLE_REQUEST_FOCUS_AUDIO
	iRet = NX_RequestCommand( NX_REQUEST_FOCUS_AUDIO_LOSS );
	if( NX_REPLY_DONE != iRet )
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_FOCUS_AUDIO_LOSS, iRet );
		return ;
	}
#endif

	iRet = NX_RequestCommand( NX_REQUEST_PROCESS_REMOVE );
	if( NX_REPLY_DONE != iRet )
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_PROCESS_REMOVE, iRet );
	}

	QMainWindow::close();
}

static void cbStatusHome( void *pObj )
{
	Q_UNUSED( pObj );

	int32_t iRet = NX_RequestCommand( NX_REQUEST_LAUNCHER_SHOW );
	if( NX_REPLY_DONE != iRet )
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_LAUNCHER_SHOW, iRet );
	}
}

static void cbStatusBack( void *pObj )
{
	AVInMainWindow* pMainWindow = (AVInMainWindow*)pObj;
	pMainWindow->close();
}
