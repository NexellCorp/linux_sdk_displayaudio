#include <QGraphicsDropShadowEffect>
#include <QGraphicsColorizeEffect>
#include <QTimer>
#include <QLinearGradient>
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <NX_DAudioUtils.h>
#include <NX_PacpClient.h>

static void cbBackGearStatus( void *pObj, int32_t iStatus )
{
	Q_UNUSED( pObj );

	int32_t iCmd = iStatus ?
		NX_REQUEST_PROCESS_HIDE | NX_REQUEST_FOCUS_VIDEO_LOSS :
		NX_REQUEST_PROCESS_SHOW | NX_REQUEST_FOCUS_VIDEO_TRANSIENT;

	int32_t iRet = NX_RequestCommand( iCmd );
	if( NX_REPLY_DONE != iRet)
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", iCmd, iRet );
		return ;
	}
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_bShowCamera( false )
{
	ui->setupUi(this);
	ui->overlayLabel->setStyleSheet("QLabel {"
									"color : rgba(1,  1, 1, 128);"
									"}");
	NX_PacpClientStart( this );
	installEventFilter( this );

	if( !NX_RearCamIsStop() )
		NX_RearCamSetStop();

	NXDA_RegisterBackGearEventCallBack( this, cbBackGearStatus );
	NXDA_StartBackGearDetectService( 163, 100 );		// ALIVE3
}

MainWindow::~MainWindow()
{
	NXDA_HideRearCam();
	removeEventFilter( this );
	NX_PacpClientStop();
	delete ui;
}

bool MainWindow::ShowCamera()
{
	if( m_bShowCamera )
		return false;

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
	m_DspInfo.iDspX			= ui->dspGraphicsView->geometry().x();
	m_DspInfo.iDspY			= ui->dspGraphicsView->geometry().y();
	m_DspInfo.iDspWidth 	= ui->dspGraphicsView->geometry().width();
	m_DspInfo.iDspHeight	= ui->dspGraphicsView->geometry().height();

	if( 0 != NXDA_ShowRearCam( &m_CamInfo, &m_DspInfo ) )
	{
		printf("Fail, NXDA_ShowRearCam().\n");
		return false;
	}

	m_bShowCamera = true;
	return true;
}

void MainWindow::HideCamera()
{
	if( !m_bShowCamera )
		return ;

	if( 0 != NXDA_HideRearCam() )
	{
		printf("Fail, NXDA_HideRearCam().\n");
	}

	m_bShowCamera = false;
}

bool MainWindow::IsShowCamera()
{
	return m_bShowCamera;
}

//
//  Dialog Event Override.
//
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
	if( event->type() == NX_QT_CUSTOM_EVENT_TYPE )
	{
		NxEvent *pEvent = reinterpret_cast<NxEvent*>(event);
		if( NX_REQUEST_PROCESS_SHOW == pEvent->m_iEventType )
		{
			if( isHidden() )
				this->show();

			ShowCamera();
			NX_PacpClientRequestRaise();
		}

		if( NX_REQUEST_PROCESS_HIDE == pEvent->m_iEventType )
		{
			NX_PacpClientRequestLower();
		}
	}

	if( (event->type() == QEvent::ActivationChange) )
	{
		printf(">>>>> NX_ReplyDone(). ( %d )\n", NX_ReplyDone() );
	}

	return QMainWindow::eventFilter(watched, event);
}

void MainWindow::showEvent( QShowEvent* event )
{
	Q_UNUSED( event );

	if( !isHidden() )
		return;

	int32_t iRet = NX_RequestCommand( NX_REQUEST_PROCESS_SHOW );
	if( NX_REPLY_DONE != iRet )
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_PROCESS_HIDE, iRet );
	}

	QMainWindow::show();
}

void MainWindow::hideEvent( QHideEvent* event )
{
	Q_UNUSED( event );

	if( isHidden() )
		return;

	int32_t iRet = NX_RequestCommand( NX_REQUEST_PROCESS_HIDE );
	if( NX_REPLY_DONE != iRet )
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_PROCESS_HIDE, iRet );
	}

	QMainWindow::hide();
}
