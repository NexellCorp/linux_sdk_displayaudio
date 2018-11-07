#include "QuickRearCamFrame.h"
#include "ui_QuickRearCamFrame.h"
#include <QTextCodec>

#include <NX_DAudioUtils.h>

#define LOG_TAG "[QuickRearCam|Frame]"
#include <NX_Log.h>


//------------------------------------------
#define NX_CUSTOM_BASE QEvent::User

static	QuickRearCamFrame *pFrame = NULL;

QuickRearCamFrame::QuickRearCamFrame(QWidget *parent)
    : QFrame(parent)
    , m_bIsInitialized(false)
    , m_pRequestTerminate(NULL)
    , m_pRequestLauncherShow(NULL)
    , m_bShowCamera(false)
    , ui(new Ui::QuickRearCamFrame)
{
    //UI Setting
    ui->setupUi(this);

    pFrame = this;

}

QuickRearCamFrame::~QuickRearCamFrame()
{
    delete ui;
}

void QuickRearCamFrame::TerminateEvent(NxTerminateEvent *)
{
    if (m_pRequestTerminate)
    {
        m_pRequestTerminate();
    }
}

void QuickRearCamFrame::RegisterRequestTerminate(void (*cbFunc)(void))
{
    if (cbFunc)
    {
        m_pRequestTerminate = cbFunc;
    }
}

void QuickRearCamFrame::RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk))
{
    if (cbFunc)
    {
        m_pRequestLauncherShow = cbFunc;
    }
}

bool QuickRearCamFrame::ShowCamera()
{
	if( m_bShowCamera )
		return false;

    NXLOGI("[%s] ShowCamera", __FUNCTION__);

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

	if( 0 != NXDA_ShowRearCam( &m_CamInfo, &m_DspInfo ) )
	{
		printf("Fail, NXDA_ShowRearCam().\n");
		return false;
	}

	m_bShowCamera = true;
	return true;
}

void QuickRearCamFrame::HideCamera()
{
	if( !m_bShowCamera )
		return ;

	if( 0 != NXDA_HideRearCam() )
	{
		printf("Fail, NXDA_HideRearCam().\n");
	}

	m_bShowCamera = false;
}

bool QuickRearCamFrame::IsShowCamera()
{
	return m_bShowCamera;
}
