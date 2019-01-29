#include "MainFrame.h"
#include "ui_MainFrame.h"
#include <sys/stat.h>
#include <sys/socket.h>
#include <QDesktopWidget>
#include <QDebug>

//#include "NX_BackGearDetect.h"
#include "NX_CCommand.h"

#define LOG_TAG "[NxRearCamMainFrame]"
#include <NX_Log.h>

#define PGL_RECT_MARGINE	10

#define DEFAULT_WIDTH	1024
#define DEFAULT_HEIGHT	600

#define STOP_COMMAND_CTRL_FILE_PATH	"/mnt/rearcam_cmd"
#define STATUS_CTRL_FILE_PATH		"/mnt/rearcam_status"

#define POLL_TIMEOUT_MS		(2000)

#define NX_CUSTOM_BASE_MAIN QEvent::User
enum {
	NX_CUSTOM_CAM_STOP = NX_CUSTOM_BASE_MAIN,
	NX_CUSTOM_CAM_START
};

static void cbBackGearStatus( void *pObj, int32_t iStatus )
{
	MainFrame *p = (MainFrame *)pObj;

	if(iStatus == 1)
	{
		p->m_bBackGearDetected = false;
		QApplication::postEvent(p, new QEvent((QEvent::Type)NX_CUSTOM_CAM_STOP));
	}else
	{
		p->m_bBackGearDetected = true;
		QApplication::postEvent(p, new QEvent((QEvent::Type)NX_CUSTOM_CAM_START));
	}
}


static void cbRearCamCommandCheckStop( int32_t status )
{
    char buf[64];
    int32_t len;

	MainFrame *pInst = (MainFrame *)MainFrame::GetInstance();

	printf("cbRearCamCommandCheckStop : Command : %d \n", status);

	if(status == STOPPED)
	{
		NX_StopCommandService(pInst->m_pCmdHandle, NULL);		

		if(pInst->backgear_enable == 1)
		{
			NX_StartBackGearDetectService( pInst->gpioIdx, 100 );		// ALIVE3
		}		
	}
}


MainFrame* MainFrame::m_spInstance = NULL;

// Launcher Show
void (*MainFrame::m_pRequestLauncherShow)(bool *bOk) = NULL;

// Message
void (*MainFrame::m_pRequestSendMessage)(const char *pDst, const char *pMsg, int32_t iMsgSize) = NULL;

// Popup Message
void (*MainFrame::m_pRequestPopupMessage)(PopupMessage *, bool *) = NULL;
void (*MainFrame::m_pReqeustExpirePopupMessage)() = NULL;

// Audio
void (*MainFrame::m_pRequestAudioFocus)(FocusPriority ePriority, bool *bOk) = NULL;
void (*MainFrame::m_pRequestAudioFocusTransient)(FocusPriority ePriority, bool *bOk) = NULL;
void (*MainFrame::m_pRequestAudioFocusLoss)(void) = NULL;

// Video
void (*MainFrame::m_pRequestVideoFocus)(FocusPriority ePriority, bool *bOk) = NULL;
void (*MainFrame::m_pRequestVideoFocusTransient)(FocusPriority ePriority, bool *bOk) = NULL;
void (*MainFrame::m_pRequestVideoFocusLoss)(void) = NULL;

// Terminate
void (*MainFrame::m_pRequestTerminate)(void) = NULL;
void (*MainFrame::m_pRequestVolume)(void) = NULL;

MainFrame::MainFrame(QWidget *parent) :
	QFrame(parent),
	m_bBackGearDetected(false),
	ui(new Ui::MainFrame)
{
	ui->setupUi(this);

	//  Initialize
	m_bInitialized = false;

	const QRect screen = QApplication::desktop()->screenGeometry();
	move(0, 0);

	if ((width() != screen.width()) || (height() != screen.height()))
	{
		setFixedSize(screen.width(), screen.height());
	}

	//  Focus
	m_bHasAudioFocus = false;
	m_bHasVideoFocus = false;
}

MainFrame::~MainFrame()
{
	delete ui;
}



bool MainFrame::event(QEvent *e)
{
	switch ((int)e->type()) {
		case NX_CUSTOM_CAM_STOP:
			RearCamStop();
			return true;

		case NX_CUSTOM_CAM_START:
			RearCamStart();
			return true;

		default:
			break;
	}
	return QFrame::event(e);
}

void MainFrame::resizeEvent(QResizeEvent *)
{
	if ((width() != DEFAULT_WIDTH) || (height() != DEFAULT_HEIGHT))
	{
		ui->m_RearCamFrame->SetupUI();
	}
}


bool MainFrame::Initialize()
{
	NXLOGI("[%s]---------- ", __FUNCTION__);
	if (m_bInitialized)
	{
		return true;
	}

	if (!m_pRequestVideoFocus)
	{
		NXLOGE("[%s] REQUEST VIDEO FOCUS does not exist.", __FUNCTION__);
		return false;
	}
	backgear_enable = 1;
	gpioIdx = 71;

	m_pIConfig = GetConfigHandle();
	GetInfo();

	if(quick_runnig == 1)
	{
		m_pCmdHandle = NX_GetCommandHandle();
		NX_RegisterCommandEventCallBack (m_pCmdHandle, cbRearCamCommandCheckStop);
		NX_StartCommandService(m_pCmdHandle, STOP_COMMAND_CTRL_FILE_PATH);
	}	

	if(backgear_enable == 1)
	{
		NXLOGI("[%s] Register BackGear Event Callback ", __FUNCTION__);
	 	NX_RegisterBackGearEventCallBack( this, cbBackGearStatus );	
	}

	hide();

	m_bInitialized = true;
	return true;
}


void MainFrame::TerminateEvent(NxTerminateEvent *)
{
	if (m_pRequestTerminate)
	{
		m_pRequestTerminate();
	}
}

MainFrame* MainFrame::GetInstance(void *pObj)
{
	if (!m_spInstance)
	{
		m_spInstance = new MainFrame((QWidget *)pObj);
	}

	return m_spInstance;
}

MainFrame* MainFrame::GetInstance()
{
	return m_spInstance;
}

void MainFrame::DestroyInstance()
{
	if (m_spInstance)
	{
		delete m_spInstance;
		m_spInstance = NULL;
	}
}

void MainFrame::RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk))
{
	if (cbFunc)
	{
		m_pRequestLauncherShow = cbFunc;
	}
}

// Message
void MainFrame::SendMessage(QString msg)
{
	NXLOGI("[%s] %s", __FUNCTION__, msg.toStdString().c_str());
}

void MainFrame::RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize))
{
	if (cbFunc)
	{
		m_pRequestSendMessage = cbFunc;
	}
}

// Audio Focus
void MainFrame::RequestAudioFocus(FocusType eType, FocusPriority ePriority, bool *bOk)
{
	if (eType == FocusType_Get)
	{
		FocusPriority eCurrPriority = FocusPriority_Normal;
		if (eCurrPriority > ePriority)
		{
			*bOk = false;
		}
		else
		{
			*bOk = true;
		}

		m_bHasAudioFocus = *bOk ? false : true;
		if(m_bHasAudioFocus == false)
		{
			 QApplication::postEvent(this, new NxTerminateEvent());
		}
	}
	else
	{
		m_bHasAudioFocus = true;
	}
}

void MainFrame::RegisterRequestAudioFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	if (cbFunc)
	{
		m_pRequestAudioFocus = cbFunc;
	}
}

void MainFrame::RequestAudioFocusTransient(FocusPriority ePriority, bool *bOk)
{
	FocusPriority eCurrPriority = FocusPriority_Normal;

	if (eCurrPriority > ePriority)
	{
		*bOk = false;
	}
	else
	{
		*bOk = true;
	}

	m_bHasAudioFocus = *bOk ? false : true;
}

void MainFrame::RegisterRequestAudioFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	if (cbFunc)
	{
		m_pRequestAudioFocusTransient = cbFunc;
	}
}

void MainFrame::RegisterRequestAudioFocusLoss(void (*cbFunc)(void))
{
	if (cbFunc)
	{
		m_pRequestAudioFocusLoss = cbFunc;
	}
}

// Video Focus
void MainFrame::RequestVideoFocus(FocusType eType, FocusPriority ePriority, bool *bOk)
{
	if (eType == FocusType_Get)
	{
		//FocusPriority eCurrPriority = FocusPriority_Normal;
		FocusPriority eCurrPriority = m_bBackGearDetected ? FocusPriority_Highest : FocusPriority_Normal;

		if (eCurrPriority > ePriority)
			*bOk = false;
		else
			*bOk = true;

		m_bHasVideoFocus = *bOk ? false : true;
	}
	else // FocusType_Set
	{

		if(m_bBackGearDetected == true)
		{
			*bOk = true;
			m_bHasVideoFocus = true;

			if (isHidden())
				show();
			raise();
		}else
		{
			*bOk = false;
			m_bHasVideoFocus = false;
			hide();
		}
	}
}

void MainFrame::RegisterRequestVideoFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	if (cbFunc)
	{
		m_pRequestVideoFocus= cbFunc;
	}
}

void MainFrame::RequestVideoFocusTransient(FocusPriority ePriority, bool *bOk)
{
	FocusPriority eCurrPriority = m_bBackGearDetected ? FocusPriority_Highest : FocusPriority_Normal;

	if (eCurrPriority > ePriority)
	{
		*bOk = false;
	}
	else
	{
		*bOk = true;
	}

	m_bHasVideoFocus = *bOk ? false : true;
}

void MainFrame::RegisterRequestVideoFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	if (cbFunc)
	{
		m_pRequestVideoFocusTransient = cbFunc;
	}
}

void MainFrame::RegisterRequestVideoFocusLoss(void (*cbFunc)(void))
{
	if (cbFunc)
	{
		m_pRequestVideoFocusLoss = cbFunc;
	}
}

void MainFrame::RegisterRequestTerminate(void (*cbFunc)(void))
{
	if (cbFunc)
	{
		m_pRequestTerminate = cbFunc;
	}
}

void MainFrame::RegisterRequestPopupMessage(void (*cbFunc)(PopupMessage *, bool *))
{
	if (cbFunc)
	{
		m_pRequestPopupMessage = cbFunc;
	}
}

void MainFrame::RegisterRequestExpirePopupMessage(void (*cbFunc)())
{
	if (cbFunc)
	{
		m_pReqeustExpirePopupMessage = cbFunc;
	}
}

void MainFrame::PopupMessageResponse(bool bOk)
{
	NXLOGI("[%s] <%s>", __FUNCTION__, bOk ? "ACCEPT" : "REJECT");
}

bool MainFrame::RearCamStart(void)
{
	bool bOk = false;
	m_pRequestVideoFocus(FocusPriority_Highest, &bOk);

	if (!bOk)
	{
		NXLOGE("[%s] REQUEST VIDEO FOCUS <FAIL>", __FUNCTION__);
		return false;
	}

	ui->m_RearCamFrame->ShowCamera();

	show();
	raise();

	return true;
}

void MainFrame::RearCamStop(void)
{
	ui->m_RearCamFrame->HideCamera();
	m_pRequestVideoFocusLoss();
	hide();

//	lower();
	m_bHasVideoFocus = false;
}



int MainFrame::GetInfo()
{
	char *pBuf = NULL;

	NXLOGI("[%s] ++++ Get RearCam Info ", __FUNCTION__);

	NX_REARCAM_INFO *vip_info;       // camera info
	DISPLAY_INFO *dsp_info;          // display info for rendering camera data
	DEINTERLACE_INFO *deinter_info;  // deinterlace info
	NX_RGB_DRAW_INFO *pgl_dsp_info;

	vip_info = &(ui->m_RearCamFrame->vip_info);
	dsp_info = &(ui->m_RearCamFrame->dsp_info);
	deinter_info = &(ui->m_RearCamFrame->deinter_info);
	pgl_dsp_info = &(ui->m_RearCamFrame->pgl_dsp_info);

	if(0 > m_pIConfig->Open("/nexell/daudio/NxRearCam/rearcam_config.xml"))
	{
		NXLOGE("[%s]xml open err\n", __FUNCTION__);
	}else
	{
		//------- camera info ---------------------------------------
		if(0 > m_pIConfig->Read("module",&pBuf))
		{
			NXLOGE("[%s]xml read module err\n", __FUNCTION__);
			vip_info->iModule 	= 1;
		}else
		{
			vip_info->iModule= atoi(pBuf);
			NXLOGI("[%s]module : %d\n", __FUNCTION__, vip_info->iModule);
		}

		if(0 > m_pIConfig->Read("use_intercam",&pBuf))
		{
			NXLOGE("[%s]xml read use_intercam err\n", __FUNCTION__);
			vip_info->bUseInterCam= 1;
		}else
		{
			vip_info->bUseInterCam = atoi(pBuf);
			NXLOGI("[%s]use_intercam : %d\n", __FUNCTION__, vip_info->bUseInterCam);
		}

		if(0 > m_pIConfig->Read("cam_width",&pBuf))
		{
			NXLOGE("[%s]xml read cam_width err\n", __FUNCTION__);
			vip_info->iWidth= 960;
		}else
		{
			vip_info->iWidth = atoi(pBuf);
			NXLOGI("[%s]cam_width : %d\n", __FUNCTION__, vip_info->iWidth);
		}

		if(0 > m_pIConfig->Read("cam_height",&pBuf))
		{
			NXLOGE("[%s]xml read cam_height err\n", __FUNCTION__);
			vip_info->iHeight= 480;
		}else
		{
			vip_info->iHeight = atoi(pBuf);
			NXLOGI("[%s]cam_widcam_heightth : %d\n", __FUNCTION__, vip_info->iHeight);
		}

		vip_info->iType			= CAM_TYPE_VIP;
		vip_info->iSensor		= nx_sensor_subdev;
		vip_info->iClipper		= nx_clipper_subdev;
		vip_info->bUseMipi		= false;

		vip_info->iCropX		= 0;
		vip_info->iCropY 		= 0;
		vip_info->iCropWidth	= vip_info->iWidth;
		vip_info->iCropHeight	= vip_info->iHeight;
		vip_info->iOutWidth 	= vip_info->iWidth;
		vip_info->iOutHeight 	= vip_info->iHeight;


		//------------display info--------------------------------

		if(0 > m_pIConfig->Read("video_layer_idx",&pBuf))
		{
			NXLOGE("[%s]xml read video_layer_idx err\n", __FUNCTION__);
			dsp_info->iPlaneIdx= 0;
		}else
		{
			dsp_info->iPlaneIdx = atoi(pBuf);
			NXLOGI("[%s]video_layer_idx : %d\n", __FUNCTION__, dsp_info->iPlaneIdx);
		}

		if(0 > m_pIConfig->Read("crtc_idx",&pBuf))
		{
			NXLOGE("[%s]xml read crtc_idx err\n", __FUNCTION__);
			dsp_info->iCrtcIdx= 0;
		}else
		{
			dsp_info->iCrtcIdx = atoi(pBuf);
			NXLOGI("[%s]crtc_idx : %d\n", __FUNCTION__, dsp_info->iCrtcIdx);
		}

		if(0 > m_pIConfig->Read("cam_display_x",&pBuf))
		{
			NXLOGE("[%s]xml read cam_display_x err\n", __FUNCTION__);
			dsp_info->iDspX= 420;
		}else
		{
			dsp_info->iDspX = atoi(pBuf);
			NXLOGI("[%s]cam_display_x : %d\n", __FUNCTION__, dsp_info->iDspX);
		}

		if(0 > m_pIConfig->Read("cam_display_y",&pBuf))
		{
			NXLOGE("[%s]xml read cam_display_y err\n", __FUNCTION__);
			dsp_info->iDspY= 0;
		}else
		{
			dsp_info->iDspY = atoi(pBuf);
			NXLOGI("[%s]cam_display_y : %d\n", __FUNCTION__, dsp_info->iDspY);
		}

		if(0 > m_pIConfig->Read("cam_display_width",&pBuf))
		{
			NXLOGE("[%s]xml read cam_display_width err\n", __FUNCTION__);
			dsp_info->iDspWidth= 1080;
		}else
		{
			dsp_info->iDspWidth = atoi(pBuf);
			NXLOGI("[%s]cam_display_width : %d\n", __FUNCTION__, dsp_info->iDspWidth);
		}

		if(0 > m_pIConfig->Read("cam_display_height",&pBuf))
		{
			NXLOGE("[%s]xml read cam_display_height err\n", __FUNCTION__);
			dsp_info->iDspHeight= 720;
		}else
		{
			dsp_info->iDspHeight = atoi(pBuf);
			NXLOGI("[%s]cam_display_height : %d\n", __FUNCTION__, dsp_info->iDspHeight);
		}

		dsp_info->uDrmFormat		= DRM_FORMAT_YUV420;
		dsp_info->iSrcWidth			= vip_info->iWidth;
		dsp_info->iSrcHeight		= vip_info->iHeight;
		dsp_info->iCropX			= 0;
		dsp_info->iCropY			= 0;
		dsp_info->iCropWidth		= vip_info->iWidth;
		dsp_info->iCropHeight		= vip_info->iHeight;

		//------------deinterlace info--------------------------------
		deinter_info->iWidth		= vip_info->iWidth;
		deinter_info->iHeight		= vip_info->iHeight;

		if(vip_info->bUseInterCam == false)
		{
			deinter_info->iEngineSel = NON_DEINTERLACER;
		}else
		{
			if(0 > m_pIConfig->Read("deinterlace_engine",&pBuf))
			{
				NXLOGE("[%s]xml read deinterlace_engine err\n", __FUNCTION__);
				deinter_info->iEngineSel= NEXELL_DEINTERLACER;
			}else
			{
				deinter_info->iEngineSel = atoi(pBuf);
				NXLOGI("[%s]deinterlace_engine : %d\n", __FUNCTION__, deinter_info->iEngineSel);
			}
		}

		if(0 > m_pIConfig->Read("deinter_param",&pBuf))
		{
			NXLOGE("[%s]xml read deinter_param err\n", __FUNCTION__);
			deinter_info->iCorr= 3;
		}else
		{
			deinter_info->iCorr = atoi(pBuf);
			NXLOGI("[%s]deinter_param : %d\n", __FUNCTION__, deinter_info->iCorr);
		}

		//-------------pgl display info ------------------------------
		int32_t lcd_w, lcd_h;

		pgl_dsp_info->m_iDspX 		= 0;
		pgl_dsp_info->m_iDspY		= 0;

		if(0 > m_pIConfig->Read("lcd_width",&pBuf))
		{
			NXLOGE("[%s]xml read lcd_width err\n", __FUNCTION__);
			lcd_w= 1920;
		}else
		{
			lcd_w = atoi(pBuf);
			NXLOGI("[%s]lcd_width : %d\n", __FUNCTION__, lcd_w);
		}

		if(0 > m_pIConfig->Read("lcd_height",&pBuf))
		{
			NXLOGE("[%s]xml read lcd_height err\n", __FUNCTION__);
			lcd_h= 720;
		}else
		{
			lcd_h = atoi(pBuf);
			NXLOGI("[%s]lcd_height : %d\n", __FUNCTION__, lcd_h);
		}

		if(0 > m_pIConfig->Read("pgl_enable",&pBuf))
		{
			NXLOGE("[%s]xml read pgl_enable err\n", __FUNCTION__);
			ui->m_RearCamFrame->pgl_enable= 1;
		}else
		{
			ui->m_RearCamFrame->pgl_enable = atoi(pBuf);
			NXLOGI("[%s]pgl_enable : %d\n", __FUNCTION__, ui->m_RearCamFrame->pgl_enable);
		}

		pgl_dsp_info->m_iDspWidth  	= lcd_w-PGL_RECT_MARGINE*2;
		pgl_dsp_info->m_iDspHeight 	= lcd_h-PGL_RECT_MARGINE*2;

		//---------------backgear info---------------------------------
		if(0 > m_pIConfig->Read("backgear_enable",&pBuf))
		{
			NXLOGE("[%s]xml read backgear_enable err\n", __FUNCTION__);
			backgear_enable= 1;
		}else
		{
			backgear_enable = atoi(pBuf);
			NXLOGI("[%s]backgear_enable : %d\n", __FUNCTION__, backgear_enable);
		}

		if(0 > m_pIConfig->Read("gpioIdx",&pBuf))
		{
			NXLOGE("[%s]xml read gpioIdx err\n", __FUNCTION__);
			gpioIdx= 71;
		}else
		{
			gpioIdx = atoi(pBuf);
			NXLOGI("[%s]gpioIdx : %d\n", __FUNCTION__, gpioIdx);
		}

		if(0 > m_pIConfig->Read("quick_running",&pBuf))
		{
			NXLOGE("[%s]xml read quick_running err\n", __FUNCTION__);
			quick_runnig = 1;
		}else
		{
			quick_runnig = atoi(pBuf);
			NXLOGI("[%s]quick_running : %d\n", __FUNCTION__, quick_runnig);
		}



		m_pIConfig->Close();
	}

	NXLOGI("[%s] ---- Get RearCam Info ", __FUNCTION__);

	return 0;
}