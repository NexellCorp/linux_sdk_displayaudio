#include "AVInFrame.h"
#include "ui_AVInFrame.h"
#include <QTextCodec>

#include <NX_DAudioUtils.h>

#define LOG_TAG "[AVIn|Frame]"
#include <NX_Log.h>


//------------------------------------------
#define NX_CUSTOM_BASE QEvent::User
enum
{
    NX_CUSTOM_BASE_ACCEPT = NX_CUSTOM_BASE+1,
    NX_CUSTOM_BASE_REJECT
};

class AcceptEvent : public QEvent
{
public:
    AcceptEvent() :
        QEvent((QEvent::Type)NX_CUSTOM_BASE_ACCEPT)
    {

    }
};

class RejectEvent : public QEvent
{
public:
    RejectEvent() :
        QEvent((QEvent::Type)NX_CUSTOM_BASE_REJECT)
    {

    }
};

////////////////////////////////////////////////////////////////////////////////
//
//	Event Callback
//
static	AVInFrame *pFrame = NULL;


//CallBack Qt
static void cbStatusHome( void *pObj )
{
    (void)pObj;
    AVInFrame *p = (AVInFrame *)pObj;
    QApplication::postEvent(p, new NxStatusHomeEvent());
}

static void cbStatusBack( void *pObj )
{
    AVInFrame *pW = (AVInFrame *)pObj;
    pW->StopAVIn();
    pW->close();
    QApplication::postEvent(pW, new NxStatusBackEvent());
}


AVInFrame::AVInFrame(QWidget *parent)
    : QFrame(parent)
    , m_bIsInitialized(false)
    , m_pStatusBar(NULL)
    , m_bButtonHide(false)
    , m_bIsVideoFocus(false)
    , m_pRequestTerminate(NULL)
    , m_pRequestLauncherShow(NULL)
    , m_bShowAVIn(false)
    , m_bGearStatus(1)
    , m_bVideoFocus(true)
    , ui(new Ui::AVInFrame)
{
    //UI Setting
    ui->setupUi(this);

    pFrame = this;

    ui->graphicsView->viewport()->installEventFilter(this);
   

    setAttribute(Qt::WA_AcceptTouchEvents, true);

    //
    //	Initialize UI Controls
    //
    //	Nexell Status Bar
    m_pStatusBar = new CNX_StatusBar( this );
    m_pStatusBar->move( 0, 0 );
    m_pStatusBar->resize( this->size().width(), this->size().height() * 1 / 10 );
    m_pStatusBar->RegOnClickedHome( cbStatusHome );
    m_pStatusBar->RegOnClickedBack( cbStatusBack );
    m_pStatusBar->SetTitleName( "Nexell AVIn" );
    
 //   ui->appNameLabel->setStyleSheet("QLabel { color : white; }");

    ShowAVIn();
 }

AVInFrame::~AVInFrame()
{
    delete ui;
}

bool AVInFrame::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->graphicsView->viewport())
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            displayTouchEvent();
        }
    }

    return QFrame::eventFilter(watched, event);
}


void AVInFrame::displayTouchEvent()
{
    if(false == m_bButtonHide)
    {
        m_bButtonHide = true;
        m_pStatusBar->hide();
    }
    else
    {
        m_pStatusBar->show();
        m_bButtonHide = false;
    }
}



bool AVInFrame::event(QEvent *event)
{
    switch ((int32_t)event->type())
    {
        case E_NX_EVENT_STATUS_HOME:
        {
            NxStatusHomeEvent *e = static_cast<NxStatusHomeEvent *>(event);
            StatusHomeEvent(e);
            return true;
        }
        case E_NX_EVENT_STATUS_BACK:
        {
            NxStatusBackEvent *e = static_cast<NxStatusBackEvent *>(event);

            StatusBackEvent(e);
            return true;
        }
         case E_NX_EVENT_TERMINATE:
        {
            NxTerminateEvent *e = static_cast<NxTerminateEvent *>(event);
            TerminateEvent(e);
            return true;
        }

        default:
            break;
    }

    return QFrame::event(event);
}

void AVInFrame::StatusHomeEvent(NxStatusHomeEvent *)
{
    if (m_pRequestLauncherShow)
    {
        bool bOk = false;
        m_pRequestLauncherShow(&bOk);
        NXLOGI("[%s] REQUEST LAUNCHER SHOW <%s>", __FUNCTION__, bOk ? "OK" : "NG");
    }
}

void AVInFrame::StatusBackEvent(NxStatusBackEvent *)
{
    QApplication::postEvent(this, new NxTerminateEvent());
}

void AVInFrame::TerminateEvent(NxTerminateEvent *)
{
    if (m_pRequestTerminate)
    {
        m_pRequestTerminate();
    }
}

void AVInFrame::RegisterRequestTerminate(void (*cbFunc)(void))
{
    if (cbFunc)
    {
        m_pRequestTerminate = cbFunc;
    }
}

void AVInFrame::RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk))
{
    if (cbFunc)
    {
        m_pRequestLauncherShow = cbFunc;
    }
}

bool AVInFrame::ShowAVIn()
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

void AVInFrame::StopAVIn()
{
 	if( !m_bShowAVIn )
		return ;

    ui->graphicsView->hide();
	NXDA_StopAVInService();	

	m_bShowAVIn = false;
}

bool AVInFrame::IsShowAVIn()
{
	return m_bShowAVIn;
}

void AVInFrame::SetVideoFocus(bool m_bVideoFocusStatus)
{
    m_bVideoFocus = m_bVideoFocusStatus;
}
