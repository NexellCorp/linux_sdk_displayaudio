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

	m_pIConfig = GetConfigHandle();

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

	m_DspInfo.iDspCrtcIdx 	= -1;
	m_DspInfo.iDspLayerIdx  = -1;
    //load crtcIdx and layerIdx
    {
        char *pBuf = NULL;
        if(0 > m_pIConfig->Open("/nexell/daudio/NxQuickRearCam/config.xml"))
		{
            NXLOGI("[%s]xml open err\n", __FUNCTION__);
            m_DspInfo.iDspCrtcIdx 	= 0;
	        m_DspInfo.iDspLayerIdx  = 0;
		}else
		{
			//load crtcIdx and layerIdx
			if(0 > m_pIConfig->Read("ctrc_idx",&pBuf))
			{
				NXLOGI("[%s]xml read ctrc_idx err\n", __FUNCTION__);
                m_DspInfo.iDspCrtcIdx 	= 0;
			}else
            {
				m_DspInfo.iDspCrtcIdx = atoi(pBuf);
            }

            if(0 > m_pIConfig->Read("layer_idx",&pBuf))
			{
				NXLOGI("[%s]xml read layer_idx err\n", __FUNCTION__);
	            m_DspInfo.iDspLayerIdx  = 0;
			}else
            {
				m_DspInfo.iDspLayerIdx = atoi(pBuf);
            }
			m_pIConfig->Close();
		}
    }
    NXLOGI("[%s]============ crtcidx : %d   layeridx : %d\n", __FUNCTION__, m_DspInfo.iDspCrtcIdx, m_DspInfo.iDspLayerIdx);
	//	Get graphic view's rect
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

	SaveInfo();

	m_bShowCamera = false;
}

bool QuickRearCamFrame::IsShowCamera()
{
	return m_bShowCamera;
}

int QuickRearCamFrame::SaveInfo()
{
    if(0 > m_pIConfig->Open("/nexell/daudio/NxQuickRearCam/config.xml"))
	{
		printf("xml open err\n");
		QFile qFile;
		qFile.setFileName("/nexell/daudio/NxQuickRearCam/config.xml");
		if(qFile.remove())
		{
			printf("config.xml is removed because of open err\n");
			if(0 > m_pIConfig->Open("/nexell/daudio/NxQuickRearCam/config.xml"))
			{
				printf("xml open err again!!\n");
				return -1;
			}
		}else
		{
			printf("Deleting config.xml is failed!\n");
			return -1;
		}
	}

    //save ctrcIdx
    {
        char pCrtcIdx[sizeof(int)] = {};
        sprintf(pCrtcIdx, "%d", m_DspInfo.iDspCrtcIdx );
        if(0 > m_pIConfig->Write("ctrc_idx", pCrtcIdx))
		{
			printf("xml write crtc index err\n");
			m_pIConfig->Close();
			return -1;
		}
    }
    //save LayerIdx
    {
        char pLayerIdx[sizeof(int)] = {};
        sprintf(pLayerIdx, "%d", m_DspInfo.iDspLayerIdx );
        if(0 > m_pIConfig->Write("layer_idx", pLayerIdx))
		{
			printf("xml write layer index err\n");
			m_pIConfig->Close();
			return -1;
		}
    }

    m_pIConfig->Close();
    return 0;
}
