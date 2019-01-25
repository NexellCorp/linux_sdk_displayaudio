#include "MainFrame.h"
#include "ui_MainFrame.h"
#include <QDebug>
#include <QDesktopWidget>

#define LOG_TAG "[NxAudioMainFrame]"
#include <NX_Log.h>

#define DEFAULT_WIDTH	1024
#define DEFAULT_HEIGHT	600

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
    ui(new Ui::MainFrame)
{
    ui->setupUi(this);

    const QRect screen = QApplication::desktop()->screenGeometry();

    if ((width() != screen.width()) || (height() != screen.height()))
    {
        setFixedSize(screen.width(), screen.height());
    }

    //  Initialize
    m_bInitialized = false;
    //  Focus
    m_bHasAudioFocus = false;
    m_bHasVideoFocus = false;

    // Register Request function
    ui->m_PlayerFrame->RegisterRequestTerminate(m_pRequestTerminate);
    ui->m_PlayerFrame->RegisterRequestLauncherShow(m_pRequestLauncherShow);
    ui->m_PlayerFrame->RegisterRequestVolume(m_pRequestVolume);
}

MainFrame::~MainFrame()
{
    delete ui;
}

bool MainFrame::Initialize()
{
    bool bOk = false;
    if (m_bInitialized)
    {
        return true;
    }

    if (!m_pRequestVideoFocus)
    {
        NXLOGE("[%s] REQUEST VIDEO FOCUS does not exist.", __FUNCTION__);
        return false;
    }

    m_pRequestVideoFocus(FocusPriority_Normal, &bOk);
    if (!bOk)
    {
        NXLOGE("[%s] REQUEST VIDEO FOCUS <FAIL>", __FUNCTION__);
        return false;
    }

    if (!m_pRequestAudioFocus)
    {
        NXLOGE("[%s] REQUEST AUDIO FOCUS does not exist.", __FUNCTION__);
        return false;
    }

    m_pRequestAudioFocus(FocusPriority_Normal, &bOk);
    if (!bOk)
    {
        NXLOGE("[%s] REQUEST AUDIO FOCUS <FAIL>", __FUNCTION__);
        return false;
    }

    if (isHidden())
    {
        show();
    }
    raise();

    m_bInitialized = true;
    return true;
}

bool MainFrame::event(QEvent *event)
{
    switch ((int32_t)event->type())
    {
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
            ui->m_PlayerFrame->StopAudio();
            ui->m_PlayerFrame->CloseAudio();
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
    qDebug() << Q_FUNC_INFO << 1;
    if (eType == FocusType_Get)
    {
        FocusPriority eCurrPriority = FocusPriority_Normal;

        if (eCurrPriority > ePriority)
            *bOk = false;
        else
            *bOk = true;
        m_bHasVideoFocus = *bOk ? false : true;

        if(!m_bHasVideoFocus)
        {
            if(0 != ui->m_PlayerFrame->GetFileListSize())
            {
                ui->m_PlayerFrame->UpdateAlbumInfo();
            }
        }
    }
    else // FocusType_Set
    {
        qDebug() << Q_FUNC_INFO << 2;
        *bOk = true;
        m_bHasVideoFocus = true;

        if (isHidden())
            show();
        raise();
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
    FocusPriority eCurrPriority  = FocusPriority_Normal;

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

void MainFrame::RegisterRequestVolume(void (*cbFunc)(void))
{
    if (cbFunc)
    {
        m_pRequestVolume = cbFunc;
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

void MainFrame::MediaEventChanged(NxMediaEvent eEvent)
{
    switch (eEvent)
    {
#if 0
        case NX_EVENT_MEDIA_SDCARD_INSERT:
        {
            NXLOGD("############### NX_EVENT_MEDIA_SDCARD_INSERT !!!\n");
            break;
        }
#endif
        case NX_EVENT_MEDIA_SDCARD_REMOVE:
        {
            NXLOGD("############### NX_EVENT_MEDIA_SDCARD_REMOVE !!!\n");
            ui->m_PlayerFrame->StorageRemoved();
            break;
        }
#if 0
        case NX_EVENT_MEDIA_USB_INSERT:
        {
            NXLOGD("############### NX_EVENT_MEDIA_USB_INSERT !!!\n");
            break;
        }
#endif
        case NX_EVENT_MEDIA_USB_REMOVE:
        {
            NXLOGD("############### NX_EVENT_MEDIA_USB_REMOVE !!!\n");
            ui->m_PlayerFrame->StorageRemoved();
            break;
        }
        case NX_EVENT_MEDIA_SCAN_DONE:
        {
            NXLOGD("############### NX_EVENT_MEDIA_SCAN_DONE !!!\n");
            ui->m_PlayerFrame->StorageScanDone();
            break;
        }
        default:
            break;
    }
}
