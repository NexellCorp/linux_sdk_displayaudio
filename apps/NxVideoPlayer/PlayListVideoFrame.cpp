#include "PlayListVideoFrame.h"
#include "ui_PlayListVideoFrame.h"

#include <QScrollBar>

static void cbStatusHome( void *pObj )
{
    (void) pObj;
    qDebug(">>>>>>>>>>>>>>>> cbStatusHome\n");
    PlayListVideoFrame *p = (PlayListVideoFrame *)pObj;
    QApplication::postEvent(p, new NxStatusHomeEvent());
}

static void cbStatusBack( void *pObj )
{
    PlayListVideoFrame *pW = (PlayListVideoFrame *)pObj;
    qDebug(">>>>>>>>>>>>>>>> cbStatusBack\n");
    pW->close();
}

static void cbStatusVolume( void *pObj )
{
    PlayListVideoFrame *pW = (PlayListVideoFrame *)pObj;
    QApplication::postEvent(pW, new NxStatusVolumeEvent());
}


PlayListVideoFrame::PlayListVideoFrame(QWidget *parent)
    : QFrame(parent)
    , m_pretIdx(-1)
    , m_selectCount(0)
    , m_pStatusBar(NULL)
    , m_pRequestLauncherShow(NULL)
    , m_pRequestVolume(NULL)
    , ui(new Ui::PlayListVideoFrame)
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
    m_pStatusBar->SetTitleName( "Nexell Video Player" );
}

PlayListVideoFrame::~PlayListVideoFrame()
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
void PlayListVideoFrame::setList(CNX_FileList *pFileList)
{
    ui->listWidget->clear();
    for( int i=0 ; i < pFileList->GetSize() ; i++ )
    {
        ui->listWidget->addItem(  strrchr( (pFileList->GetList(i)).toStdString().c_str(), '/' ) +1 );
    }
}

void PlayListVideoFrame::setCurrentIndex(int idx)
{
    ui->listWidget->setCurrentRow(idx);
}

int PlayListVideoFrame::getCurrentIndex()
{
    return m_selectIdx.row();
}

//
//	Button Event Control
//
void PlayListVideoFrame::on_btnCancel_released()
{
    lower();
    emit signalPlayListReject();
}

void PlayListVideoFrame::on_btnOk_released()
{
    lower();
    emit signalPlayListAccept();
}

void PlayListVideoFrame::on_listWidget_itemClicked(QListWidgetItem *item)
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

bool PlayListVideoFrame::event(QEvent *e)
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

void PlayListVideoFrame::RegisterRequestVolume(void (*cbFunc)(void))
{
    if (cbFunc)
    {
        m_pRequestVolume = cbFunc;
    }
}

void PlayListVideoFrame::RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk))
{
    if (cbFunc)
    {
        m_pRequestLauncherShow = cbFunc;
    }
}
