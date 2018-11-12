#include "MainFrame.h"
#include "ui_MainFrame.h"
#include <QDebug>

#define LOG_TAG "[NxQuickRearCamMainFrame]"
#include <NX_Log.h>

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
    //  Focus
    m_bHasAudioFocus = false;
    m_bHasVideoFocus = false;

    // Register Request function
    // ui->m_QuickRearCamFrame->RegisterRequestTerminate(m_pRequestTerminate);
    //ui->m_QuickRearCamFrame->RegisterRequestLauncherShow(m_pRequestLauncherShow);
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

bool MainFrame::Initialize()
{
    if (m_bInitialized)
    {
        return true;
    }

    if (!m_pRequestVideoFocus)
    {
        NXLOGE("[%s] REQUEST VIDEO FOCUS does not exist.", __FUNCTION__);
        return false;
    }

    if( !NX_RearCamIsStop() )
		NX_RearCamSetStop();

	NXDA_RegisterBackGearEventCallBack( this, cbBackGearStatus );
	NXDA_StartBackGearDetectService( 163, 100 );		// ALIVE3

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

    ui->m_QuickRearCamFrame->ShowCamera();

    show();
    raise();

    return true;
}

void MainFrame::RearCamStop(void)
{
    ui->m_QuickRearCamFrame->HideCamera();
    m_pRequestVideoFocusLoss();
    hide();

//	lower();
    m_bHasVideoFocus = false;
}
