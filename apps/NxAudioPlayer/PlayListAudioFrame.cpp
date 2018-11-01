#include "PlayListAudioFrame.h"
#include "ui_PlayListAudioFrame.h"

#include <QScrollBar>

static void cbStatusHome( void *pObj )
{
    (void) pObj;
    qDebug("List >>>>>>>>>>>>>>>> cbStatusHome\n");
    PlayListAudioFrame *p = (PlayListAudioFrame *)pObj;
    QApplication::postEvent(p, new NxStatusHomeEvent());
}

static void cbStatusBack( void *pObj )
{
    PlayListAudioFrame *pW = (PlayListAudioFrame *)pObj;
    qDebug(">>>>>>>>>>>>>>>> cbStatusBack\n");
    pW->close();
}

static void cbStatusVolume( void *pObj )
{
    PlayListAudioFrame *pW = (PlayListAudioFrame *)pObj;
    QApplication::postEvent(pW, new NxStatusVolumeEvent());
}


PlayListAudioFrame::PlayListAudioFrame(QWidget *parent)
    : QFrame(parent)
    , m_pretIdx(-1)
    , m_selectCount(0)
    , m_pStatusBar(NULL)
    , m_pRequestLauncherShow(NULL)
    , m_pRequestVolume(NULL)
    , ui(new Ui::PlayListAudioFrame)
{
	ui->setupUi(this);

    memset(&m_selectIdx,0,sizeof(QModelIndex) );

    ui->listWidget->setStyleSheet(
        "QListWidget{"
            "padding-left : 20px;"
        "}"
        "QListWidget::item {"
            "padding : 20px;"
        "}"
    );

    ui->listWidget->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical {"
            "width: 50px;"
            "min-height: 100px;"
        "}"
    );

    m_pStatusBar = new CNX_StatusBar( this );
    m_pStatusBar->move( 0, 0 );
    m_pStatusBar->resize( this->size().width(), this->size().height() * 1 / 10 );
    m_pStatusBar->RegOnClickedHome( cbStatusHome );
    m_pStatusBar->RegOnClickedBack( cbStatusBack );
    m_pStatusBar->RegOnClickedVolume( cbStatusVolume );
    m_pStatusBar->SetTitleName( "Nexell Audio Player" );
}

PlayListAudioFrame::~PlayListAudioFrame()
{
    if(m_pStatusBar)
    {
        delete m_pStatusBar;
    }
	delete ui;
}

//
//	List Index Control
//
void PlayListAudioFrame::setList(CNX_FileList *pFileList)
{
    ui->listWidget->clear();
    for( int i=0 ; i < pFileList->GetSize() ; i++ )
    {
        ui->listWidget->addItem(  strrchr( (pFileList->GetList(i)).toStdString().c_str(), '/' ) +1 );
    }
}

void PlayListAudioFrame::setCurrentIndex(int idx)
{
    ui->listWidget->setCurrentRow(idx);
}

int PlayListAudioFrame::getCurrentIndex()
{
    return m_selectIdx.row();
}

//
//	Button Event Control
//
void PlayListAudioFrame::on_btnCancel_released()
{
    lower();
    emit signalPlayListReject();
}

void PlayListAudioFrame::on_btnOk_released()
{
    lower();
    emit signalPlayListAccept();
}

void PlayListAudioFrame::on_listWidget_itemClicked(QListWidgetItem *item)
{
    m_selectCount++;
    m_selectIdx = item->listWidget()->currentIndex();
    if(2 == m_selectCount)
    {
        if(m_pretIdx == m_selectIdx.row())
        {
            emit signalPlayListAccept();
        }
    }
    else
    {
        m_selectCount = 1;
    }
    m_pretIdx = m_selectIdx.row();
}

bool PlayListAudioFrame::event(QEvent *e)
{
    switch ((int32_t)e->type())
    {
        case E_NX_EVENT_STATUS_HOME:
        {
            if (m_pRequestLauncherShow)
            {
                bool bOk = false;
                m_pRequestLauncherShow(&bOk);
                if (bOk)
                {
                }
            }
            return true;
        }
        case E_NX_EVENT_STATUS_VOLUME:
        {
            if (m_pRequestVolume)
            {
                m_pRequestVolume();
            }
            return true;
        }

        default:
            break;
    }

    return QFrame::event(e);
}

void PlayListAudioFrame::RegisterRequestVolume(void (*cbFunc)(void))
{
    if (cbFunc)
    {
        m_pRequestVolume = cbFunc;
    }
}

void PlayListAudioFrame::RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk))
{
    if (cbFunc)
    {
        m_pRequestLauncherShow = cbFunc;
    }
}
