#include "PlayerVideoFrame.h"
#include "ui_PlayerVideoFrame.h"
#include <QTextCodec>
#include <QDesktopWidget>

//Display Mode
#define DSP_FULL   0
#define DSP_HALF   1

//Display Info
#define DSP_FULL_WIDTH  1024
#define DSP_FULL_HEIGHT 600
#define DSP_HALF_WIDTH  DSP_FULL_WIDTH/2
#define DSP_HALF_HEIGHT 600

//Button Width,Height
#define BUTTON_Y            520
#define BUTTON_FULL_X       20
#define BUTTON_FULL_WIDTH   90
#define BUTTON_FULL_HEIGHT  60
#define BUTTON_HALF_X       10
#define BUTTON_HALF_WIDTH   40
#define BUTTON_HALF_HEIGHT  60

#define LOG_TAG "[VideoPlayer|Frame]"
#include <NX_Log.h>

#define DEFAULT_DSP_WIDTH	1024
#define DEFAULT_DSP_HEIGHT	600

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
static	CallBackSignal mediaStateCb;
static	PlayerVideoFrame *pPlayFrame = NULL;

//CallBack Eos, Error
static void cbEventCallback( void */*privateDesc*/, unsigned int EventType, unsigned int /*EventData*/, unsigned int /*param*/ )
{
	mediaStateCb.statusChanged(EventType);
}

//CallBack Qt
static void cbStatusHome( void *pObj )
{
	(void)pObj;
	PlayerVideoFrame *p = (PlayerVideoFrame *)pObj;
	QApplication::postEvent(p, new NxStatusHomeEvent());
}

static void cbStatusBack( void *pObj )
{
	PlayerVideoFrame *pW = (PlayerVideoFrame *)pObj;
	pW->SaveInfo();
	pW->StopVideo();
	pW->close();
	QApplication::postEvent(pW, new NxStatusBackEvent());
}

static void cbStatusVolume( void *pObj )
{
	PlayerVideoFrame *pW = (PlayerVideoFrame *)pObj;
	QApplication::postEvent(pW, new NxStatusVolumeEvent());
}

static int32_t cbSqliteRowCallback( void *pObj, int32_t iColumnNum, char **ppColumnValue, char **ppColumnName )
{
	char* cFileType = NULL;
	char* cFilePath = NULL;
	for( int32_t i = 0; i < iColumnNum; i++ )
	{
		if( !strcmp("_type", ppColumnName[i] ))
		{
			cFileType  = ppColumnValue[i];
		}

		if( !strcmp("_path", ppColumnName[i] ))
		{
			cFilePath = ppColumnValue[i];
		}
	}

	//if(type == video)-->add
	if( !strcmp("video", cFileType ))
	{
		(((PlayerVideoFrame*)pObj)->GetFileList())->AddItem( QString::fromUtf8(cFilePath) );
	}
	return 0;
}

PlayerVideoFrame::PlayerVideoFrame(QWidget *parent)
	: QFrame(parent)
	, m_pNxPlayer(NULL)
	, m_bSubThreadFlag(false)
	, m_iVolValue(30)
	, m_iDuration(0)
	, m_bIsInitialized(false)
	, m_pStatusBar(NULL)
	, m_bSeekReady(false)
	, m_bButtonHide(false)
	, m_iCurFileListIdx (0)
	, m_bTurnOffFlag(false)
	, m_bStopRenderingFlag(false)
	, m_bTryFlag(false)
	, m_pPlayListFrame(NULL)
	, m_bIsVideoFocus(true)
	, m_bIsAudioFocus(true)
	, m_pRequestTerminate(NULL)
	, m_pRequestLauncherShow(NULL)
	, m_pRequestVolume(NULL)
	, m_fSpeed(1.0)
	, m_bNotSupportSpeed (false)
	, m_pMessageFrame(NULL)
	, m_pMessageLabel(NULL)
	, m_pMessageButton(NULL)
	, ui(new Ui::PlayerVideoFrame)
{
	//UI Setting
	ui->setupUi(this);

	const QRect screen = QApplication::desktop()->screenGeometry();
	if ((width() != screen.width()) || (height() != screen.height()))
	{
		setFixedSize(screen.width(), screen.height());
	}


	m_pNxPlayer = new CNX_MoviePlayer();
	UpdateFileList();
	m_pIConfig = GetConfigHandle();

	//	Connect Solt Functions
	connect(&mediaStateCb, SIGNAL(mediaStatusChanged(int)), SLOT(statusChanged(int)));
	pPlayFrame = this;

	ui->graphicsView->viewport()->installEventFilter(this);
	ui->progressBar->installEventFilter(this);

	//Update position timer
	connect(&m_PosUpdateTimer, SIGNAL(timeout()), this, SLOT(DoPositionUpdate()));
	//Update Subtitle
	connect(&m_PosUpdateTimer, SIGNAL(timeout()), this, SLOT(subTitleDisplayUpdate()));

	setAttribute(Qt::WA_AcceptTouchEvents, true);

	m_PosUpdateTimer.start(300);

	//
	//	Initialize UI Controls
	//
	//	Nexell Status Bar
	m_pStatusBar = new CNX_StatusBar( this );
	m_pStatusBar->move( 0, 0 );
	m_pStatusBar->resize( this->size().width(), this->size().height() * 1 / 10 );
	m_pStatusBar->RegOnClickedHome( cbStatusHome );
	m_pStatusBar->RegOnClickedBack( cbStatusBack );
	m_pStatusBar->RegOnClickedVolume( cbStatusVolume );
	m_pStatusBar->SetTitleName( "Nexell Video Player" );

	ui->durationlabel->setStyleSheet("QLabel { color : white; }");
	ui->appNameLabel->setStyleSheet("QLabel { color : white; }");
	ui->subTitleLabel->setStyleSheet("QLabel { color : white; }");
	ui->subTitleLabel2->setStyleSheet("QLabel { color : white; }");

	//Message
	m_pMessageFrame = new QFrame(this);

	m_pMessageFrame->setGeometry(340, 190, 271, 120);
	m_pMessageFrame->setStyleSheet("background: white;");
	m_pMessageFrame->hide();

	m_pMessageLabel = new QLabel(m_pMessageFrame);
	m_pMessageLabel->setGeometry(0, 0, m_pMessageFrame->width(), 100);
	m_pMessageLabel->setText("text");

	m_pMessageButton = new QPushButton(m_pMessageFrame);
	m_pMessageButton->setGeometry(m_pMessageFrame->width()/2-100/2, m_pMessageFrame->height()-30, 80, 30);
	m_pMessageButton->setText("Ok");
	connect(m_pMessageButton, SIGNAL(clicked(bool)), this, SLOT(slotOk()));

	//Get audioDeviceName
	memset(m_audioDeviceName,0,sizeof(m_audioDeviceName));
	if(0 > m_pIConfig->Open("/nexell/daudio/NxVideoPlayer/nxvideoplayer_config.xml"))
	{
		NXLOGE("[%s]nxvideooplayer_config.xml open err\n", __FUNCTION__);
	}
	else
	{
		char *pBuf = NULL;
		if(0 > m_pIConfig->Read("alsa_default",&pBuf))
		{
			NXLOGE("[%s]xml alsa_default err\n", __FUNCTION__);
			memcpy(m_audioDeviceName,"plughw:0,0",sizeof("plughw:0,0"));
		}
		else
		{
			strcpy(m_audioDeviceName,pBuf);
		}
	}
	m_pIConfig->Close();
}

PlayerVideoFrame::~PlayerVideoFrame()
{
	if(m_pNxPlayer)
	{
		NX_MediaStatus state = m_pNxPlayer->GetState();
		if( (PlayingState == state)||(PausedState == state) )
		{
			StopVideo();
		}

		delete m_pNxPlayer;
		m_pNxPlayer = NULL;
	}
	if(m_pPlayListFrame)
	{
		delete m_pPlayListFrame;
		m_pPlayListFrame = NULL;
	}

	if(m_pStatusBar)
	{
		delete m_pStatusBar;
	}

	if(m_pMessageButton)
	{
		delete m_pMessageButton;
	}

	if(m_pMessageLabel)
	{
		delete m_pMessageLabel;
	}

	if(m_pMessageFrame)
	{
		delete m_pMessageFrame;
	}

	delete ui;
}

//
//	xml (save previous state)
int32_t PlayerVideoFrame::SaveInfo()
{
	if( NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return -1;
	}

	if(0 > m_pIConfig->Open("/nexell/daudio/NxVideoPlayer/config.xml"))
	{
		NXLOGW("xml open err\n");
		QFile qFile;
		qFile.setFileName("/nexell/daudio/NxVideoPlayer/config.xml");
		if(qFile.remove())
		{
			NXLOGW("config.xml is removed because of open err\n");
			if(0 > m_pIConfig->Open("/nexell/daudio/NxVideoPlayer/config.xml"))
			{
				NXLOGE("xml open err again!!\n");
				return -1;
			}
		}else
		{
			NXLOGE("Deleting config.xml is failed!\n");
			return -1;
		}
	}

	//save current media path
	m_listMutex.Lock();
	QString curPath = m_FileList.GetList(m_iCurFileListIdx);
	m_listMutex.Unlock();
	if( curPath.isEmpty() || curPath.isNull() )
	{
		NXLOGE("current path is not valid\n");
		m_pIConfig->Close();
		return -1;
	}

	// encode pCurPath(QString , unicode) to UTF-8
	QTextCodec* pCodec = QTextCodec::codecForName("UTF-8");		//pCodec  271752
	QTextEncoder* pEncoder = pCodec->makeEncoder();
	QByteArray encodedByteArray = pEncoder->fromUnicode(curPath);
	char* pCurPath = (char*)encodedByteArray.data();

	if(0 > m_pIConfig->Write("path", pCurPath ))
	{
		NXLOGE("xml write path err\n");
		m_pIConfig->Close();
		return -1;
	}

	//save current media position
	if( (PlayingState == m_pNxPlayer->GetState())||(PausedState == m_pNxPlayer->GetState()) )
	{
		char pCurPos[sizeof(long long int)] = {};
		qint64 iCurPos = m_pNxPlayer->GetMediaPosition();
		if( 0 > iCurPos)
		{
			NXLOGW("current position is not valid  iCurPos : %lld is set to 0\n",iCurPos);
			iCurPos = 0;
		}
		sprintf(pCurPos, "%lld", iCurPos);
		if(0 > m_pIConfig->Write("pos", pCurPos))
		{
			NXLOGE("xml write pos err\n");
			m_pIConfig->Close();
			return -1;
		}
	}else if(StoppedState == m_pNxPlayer->GetState())
	{
		char pCurPos[sizeof(int)] = {};
		sprintf(pCurPos, "%d", 0);
		if(0 > m_pIConfig->Write("pos", pCurPos))
		{
			NXLOGE("xml write pos err\n");
			m_pIConfig->Close();
			return -1;
		}
	}

	m_pIConfig->Close();
	return 0;
}

bool PlayerVideoFrame::SeekToPrev(int* iSavedPosition, int* iFileIdx)
{
	if( NULL == m_pNxPlayer)
	{
		NXLOGE("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}

	if(0 > m_pIConfig->Open("/nexell/daudio/NxVideoPlayer/config.xml"))
	{
		NXLOGE("xml open err\n");
		QFile qFile;
		qFile.setFileName("/nexell/daudio/NxVideoPlayer/config.xml");
		if(qFile.remove())
		{
			NXLOGE("config.xml is removed because of open err\n");
		}else
		{
			NXLOGE("Deleting config.xml is failed!\n");
		}
		return false;
	}

	//load previous media path
	char* pPrevPath = NULL;
	if(0 > m_pIConfig->Read("path",&pPrevPath))
	{
		NXLOGE("xml read path err\n");
		m_pIConfig->Close();
		return false;
	}

	//load previous media position
	char* pBuf = NULL;
	if(0 > m_pIConfig->Read("pos",&pBuf))
	{
		NXLOGE("xml read pos err\n");
		m_pIConfig->Close();
		return false;
	}
	*iSavedPosition = atoi(pBuf);
	m_pIConfig->Close();

	//find index in file list by path
	m_listMutex.Lock();
	if(0 < m_FileList.GetSize())
	{
		//media file list is valid
		//find pPrevPath in list

		int iIndex = m_FileList.GetPathIndex(QString::fromUtf8(pPrevPath));
		if(0 > iIndex)
		{
			NXLOGE("saved path does not exist in FileList\n");
			m_listMutex.Unlock();
			return false;
		}
		*iFileIdx = iIndex;
		m_listMutex.Unlock();
		return true;
	}else
	{
		NXLOGD("File List is not valid.. no media file or media scan is not done\n");
		NXLOGD("just try last path : %s\n\n",pPrevPath);
		m_bTryFlag = true;
		m_FileList.AddItem(QString::fromUtf8(pPrevPath));
		*iFileIdx = 0;
		m_listMutex.Unlock();
		return true;
	}

	return false;
}

//
//	Storage Event
void PlayerVideoFrame::StorageRemoved()
{
	m_bTurnOffFlag = true;

	if(NULL != m_pNxPlayer)
	{
		SaveInfo();
		StopVideo();
	}else
	{
		NXLOGE("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
	}

	if(NULL != m_pPlayListFrame)
	{
		m_pPlayListFrame->close();
	}

	qDebug("########## StorageRemoved()\n");
	//    this->close();
	QApplication::postEvent(this, new NxTerminateEvent());
}

void PlayerVideoFrame::StorageScanDone()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGE("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return;
	}
	m_listMutex.Lock();
	bool bPlayFlag = false;
	if(0 == m_FileList.GetSize())
	{
		//videoplayer is obviously not playing..
		UpdateFileList();
		if(0 != m_FileList.GetSize())
		{
			bPlayFlag = true;
		}
	}else
	{
		//videoplayer could be playing some file...
		//what if file list is accessed some where...
		//m_listMutex.....prev next play seekToPrev SaveInfo StorageRemoved StorageScanDone
		QString curPath = m_FileList.GetList(m_iCurFileListIdx);
		m_FileList.ClearList();
		m_iCurFileListIdx = 0;
		UpdateFileList();

		if( NULL != curPath)
		{
			int iIndex = m_FileList.GetPathIndex(curPath);
			if(0 > iIndex)
			{
				NXLOGD("line : %d , path does not exist in FileList\n",__LINE__);
				m_iCurFileListIdx = 0;
			}else
			{
				m_iCurFileListIdx = iIndex;
			}
		}
	}
	m_listMutex.Unlock();

	if(NULL != m_pPlayListFrame)
	{
		m_listMutex.Lock();
		m_pPlayListFrame->setList(&m_FileList);
		m_listMutex.Unlock();
	}

	if(bPlayFlag)
	{
		PlaySeek();
	}
}

void PlayerVideoFrame::UpdateFileList()
{
	//	read data base that Media Scaning made.
	char szPath[256];
	snprintf( szPath, sizeof(szPath), "%s/%s", NX_MEDIA_DATABASE_PATH, NX_MEDIA_DATABASE_NAME );
	NX_SQLiteGetData( szPath, NX_MEDIA_DATABASE_TABLE, cbSqliteRowCallback, (void*)this);
	NXLOGD("<<< Total file list = %d\n", m_FileList.GetSize());
}

CNX_FileList *PlayerVideoFrame::GetFileList()
{
	return &m_FileList;
}

bool PlayerVideoFrame::eventFilter(QObject *watched, QEvent *event)
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
	else if (watched == ui->progressBar)
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>(event);
			updateProgressBar(pMouseEvent, false);
		}
		else if (event->type() == QEvent::MouseButtonRelease)
		{
			QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>(event);
			updateProgressBar(pMouseEvent, true);
		}
	}

	return QFrame::eventFilter(watched, event);
}

////////////////////////////////////////////////////////////////////
//
//      Update Player Progress Bar & Volume Progress Bar
//
////////////////////////////////////////////////////////////////////
void PlayerVideoFrame::updateProgressBar(QMouseEvent *event, bool bReleased)
{
	if( bReleased )
	{
		//	 Do Seek
		if( m_bSeekReady )
		{
			if( StoppedState !=m_pNxPlayer->GetState() )
			{
				double ratio = (double)event->x()/ui->progressBar->width();
				qint64 position = ratio * m_iDuration;
				SeekVideo( position );

				//seek subtitle
				ui->subTitleLabel->setText("");
				ui->subTitleLabel2->setText("");
				m_pNxPlayer->SeekSubtitle(position);

				NXLOGD("Position = %lld", position);
			}
			NXLOGD("Do Seek !!!");
			m_PosUpdateTimer.start(300);
		}
		m_bSeekReady = false;
	}
	else
	{
		m_bSeekReady = true;
		NXLOGD("Ready to Seek\n");
	}
}

void PlayerVideoFrame::setVideoFocus(bool bVideoFocus)
{
	m_bIsVideoFocus = bVideoFocus;
}

void PlayerVideoFrame::setAudioFocus(bool bAudioFocus)
{
	m_bIsAudioFocus = bAudioFocus;
}

void PlayerVideoFrame::on_prevButton_released()
{
	if(m_bIsAudioFocus == false)
	{
		return;
	}

	PlayPreviousVideo();
}

void PlayerVideoFrame::on_playButton_released()
{
	if(m_bIsAudioFocus == false)
	{
		return;
	}

	PlayVideo();
}

void PlayerVideoFrame::on_pauseButton_released()
{
	if(m_bIsAudioFocus == false)
	{
		return;
	}

	PauseVideo();
}

void PlayerVideoFrame::on_nextButton_released()
{
	if(m_bIsAudioFocus == false)
	{
		return;
	}

	PlayNextVideo();
}

void PlayerVideoFrame::on_stopButton_released()
{
	if(m_bIsAudioFocus == false)
	{
		return;
	}

	StopVideo();
}

void PlayerVideoFrame::DoPositionUpdate()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		ui->progressBar->setRange(0, 100);
		ui->progressBar->setValue(0);
		UpdateDurationInfo( 0, 0 );
		return;
	}
	if(m_bIsInitialized)
	{
		int64_t iDuration = m_pNxPlayer->GetMediaDuration();
		int64_t iPosition = m_pNxPlayer->GetMediaPosition();

		if( (0 > iDuration) || (0 > iPosition) )
		{
			iPosition = 0;
			iDuration = 0;
		}
		//	ProgressBar
		ui->progressBar->setValue(iPosition /1000 );
		UpdateDurationInfo( iPosition/1000, iDuration/1000 );
	}else
	{
		ui->progressBar->setRange(0, 100);
		ui->progressBar->setValue(0);
		UpdateDurationInfo( 0, 0 );
	}
}

void PlayerVideoFrame::UpdateDurationInfo(int64_t position, int64_t duration)
{
	QString tStr;
	QTime currentTime((position/3600)%60, (position/60)%60, position%60, (position*1000)%1000);
	QTime totalTime((duration/3600)%60, (duration/60)%60, duration%60, (duration*1000)%1000);
	QString format = "mm:ss";
	if (duration >= 3600)
	{
		format = "hh:mm:ss";
	}
	tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
	ui->durationlabel->setText(tStr);
	//qDebug(">> %s() Out %s", __func__, tStr.toStdString().c_str());
}

void PlayerVideoFrame::statusChanged(int eventType)
{
	if(m_bTurnOffFlag)
	{
		NXLOGW("%s  , line : %d close app soon.. bypass \n", __FUNCTION__, __LINE__);
		return;
	}
	switch (eventType)
	{
	case MP_MSG_EOS:
	{
		NXLOGI("******** EndOfMedia !!!\n");
		PlayNextVideo();
		break;
	}
	case MP_MSG_DEMUX_ERR:
	{
		NXLOGW("******** MP_MSG_DEMUX_ERR !!!\n");
		// message
		m_pMessageFrame->show();
		m_pMessageLabel->setText("DEMUX_ERR");

		ui->progressBar->setValue(0);
		UpdateDurationInfo( 0, 0 );
		StopVideo();
		break;
	}
	case MP_MSG_ERR_OPEN_AUDIO_DEVICE:
	{
		NXLOGE("******** MP_MSG_ERR_OPEN_AUDIO_DEVICE\n");
		// message
		m_pMessageFrame->show();
		m_pMessageLabel->setText("ERR_OPEN_AUDIO_DEVICE");

		ui->progressBar->setValue(0);
		UpdateDurationInfo( 0, 0 );
		StopVideo();
		break;
	}

	default:
		break;
	}
}

bool PlayerVideoFrame::StopVideo()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}

	if(0 == m_FileList.GetSize())
	{
		NXLOGW("%s(), line: %d, m_FileList is 0 \n", __FUNCTION__, __LINE__);
		return false;
	}

	m_statusMutex.Lock();
	m_bStopRenderingFlag = true;
	m_statusMutex.Unlock();

	m_pNxPlayer->Stop();
	CloseVideo();

	m_fSpeed = 1.0;
	ui->speedButton->setText("x 1");
	QString style;
	style += "QProgressBar {";
	style += "  border: 2px solid grey;";
	style += "  border-radius: 5px;";
	style += "  background: white;";
	style += "}";

	style += "QProgressBar::chunk {";
	style += "  background-color: rgb(37, 86, 201);";
	style += "width: 20px;";
	style += "}";
	ui->progressBar->setStyleSheet(style);

	if(m_bNotSupportSpeed)
	{
		m_bNotSupportSpeed = false;
		m_pMessageFrame->hide();
	}

	return true;
}

bool PlayerVideoFrame::CloseVideo()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}
	m_bIsInitialized = false;

	StopSubTitle();

	if(0 > m_pNxPlayer->CloseHandle())
	{
		NXLOGE("%s(), line: %d, CloseHandle failed \n", __FUNCTION__, __LINE__);
		return false;
	}
	return true;
}

bool PlayerVideoFrame::SetVideoVolume( int32_t volume )
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}

	m_pNxPlayer->SetVolume(volume);

	if( m_pStatusBar )
	{
		m_pStatusBar->SetVolume(volume);
	}
	return true;
}

bool PlayerVideoFrame::PlayNextVideo()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}

	StopVideo();

	//	find next index
	if(0 != m_FileList.GetSize())
	{
		m_iCurFileListIdx = (m_iCurFileListIdx+1) % m_FileList.GetSize();
	}
	return PlayVideo();
}

bool PlayerVideoFrame::PlayPreviousVideo()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}

	StopVideo();

	//	Find previous index
	if(0 != m_FileList.GetSize())
	{
		m_iCurFileListIdx --;
		if( 0 > m_iCurFileListIdx )
			m_iCurFileListIdx = m_FileList.GetSize() -1;
	}
	return PlayVideo();
}

void PlayerVideoFrame::PlaySeek()
{
	bool seekflag = false;
	int iSavedPosition = 0;

	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return;
	}

	seekflag = SeekToPrev(&iSavedPosition, &m_iCurFileListIdx);

	PlayVideo();

	if(seekflag)
	{
		//seek video
		SeekVideo( iSavedPosition );

		//seek subtitle
		ui->subTitleLabel->setText("");
		ui->subTitleLabel2->setText("");
		m_pNxPlayer->SeekSubtitle(iSavedPosition);
	}
}

bool PlayerVideoFrame::PlayVideo()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}

	if(PlayingState == m_pNxPlayer->GetState())
	{
		NXLOGW("already playing\n");
		return true;
	}

	if( (PausedState == m_pNxPlayer->GetState()) || (ReadyState == m_pNxPlayer->GetState()))
	{
		m_pNxPlayer->Play();

		if(1.0 == m_pNxPlayer->GetVideoSpeed())
		{
			m_fSpeed = 1.0;
			ui->speedButton->setText("x 1");
			QString style;
			style += "QProgressBar {";
			style += "  border: 2px solid grey;";
			style += "  border-radius: 5px;";
			style += "  background: white;";
			style += "}";

			style += "QProgressBar::chunk {";
			style += "  background-color: rgb(37, 86, 201);";
			style += "width: 20px;";
			style += "}";

			ui->progressBar->setStyleSheet(style);
		}
		return true;
	}
	else if(StoppedState == m_pNxPlayer->GetState())
	{
		m_listMutex.Lock();
		if( 0 < m_FileList.GetSize() )
		{
			int iResult = -1;
			int iTryCount = 0;
			int iMaxCount = m_FileList.GetSize();
			while(0 > iResult)
			{
				if(m_bIsVideoFocus)
				{
					m_statusMutex.Lock();
					m_bStopRenderingFlag = false;
					m_statusMutex.Unlock();
				}

				m_fSpeed = 1.0;
				ui->speedButton->setText("x 1");

				iResult = m_pNxPlayer->InitMediaPlayer( cbEventCallback,
														NULL,
														m_FileList.GetList(m_iCurFileListIdx).toStdString().c_str(),
														MP_TRACK_VIDEO,
														width(),
														height(),
														m_audioDeviceName,
														//&cbUpdateRender
														NULL
														);

				iTryCount++;
				if(0 == iResult)
				{
					NXLOGI("\n\n********************media init done!\n");
					m_bIsInitialized = true;

					if( 0 == OpenSubTitle() )
					{
						PlaySubTitle();
					}

					if( 0 > m_pNxPlayer->Play() )
					{
						NXLOGE("NX_MPPlay() failed !!!");
						iResult = -1; //retry with next file..
					}else
					{
						//play succeed
						//Update Applications
						//                        SetVideoVolume(m_iVolValue);

						m_iDuration = m_pNxPlayer->GetMediaDuration();
						ui->progressBar->setMaximum( m_iDuration/1000 );	//	Max Seconds
						ui->appNameLabel->setText(m_FileList.GetList(m_iCurFileListIdx));

						if(1.0 == m_pNxPlayer->GetVideoSpeed())
						{
							ui->speedButton->setText("x 1");
							QString style;
							style += "QProgressBar {";
							style += "  border: 2px solid grey;";
							style += "  border-radius: 5px;";
							style += "  background: white;";
							style += "}";

							style += "QProgressBar::chunk {";
							style += "  background-color: rgb(37, 86, 201);";
							style += "width: 20px;";
							style += "}";

							ui->progressBar->setStyleSheet(style);
						}

						NXLOGI("********************media play done!\n\n");
						m_listMutex.Unlock();
						return true;
					}
				}

				if(m_bTryFlag)
				{
					//This case is for playing back to last file
					//When there is no available file list but last played file path exists in config.xml,
					//videoplayer tries playing path that saved in config.xml .
					//If trying playing is failed, videoplayer should stop trying next file.
					NXLOGI( "Have no available contents!!\n" );
					m_bTryFlag = false;
					m_FileList.ClearList();
					m_listMutex.Unlock();
					return false;
				}

				if( iTryCount == iMaxCount )
				{
					//all list is tried, but nothing succeed.
					NXLOGI( "Have no available contents!!\n" );
					m_listMutex.Unlock();
					return false;
				}

				NXLOGW("MediaPlayer Initialization fail.... retry with next file\n");
				m_iCurFileListIdx = (m_iCurFileListIdx+1) % m_FileList.GetSize();
				CloseVideo();
			}
		}else
		{
			NXLOGW( "Have no available contents!! InitMediaPlayer is not tried \n" );
			m_listMutex.Unlock();
			return false;
		}
	}
	return true;
}

bool PlayerVideoFrame::PauseVideo()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGE("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}

	if(StoppedState == m_pNxPlayer->GetState() ||
			ReadyState == m_pNxPlayer->GetState()
			)
	{
		return false;
	}
	if(0 > m_pNxPlayer->Pause())
		return false;

	return true;
}

bool PlayerVideoFrame::SeekVideo( int32_t mSec )
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGE("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}

	if(PlayingState == m_pNxPlayer->GetState() || PausedState == m_pNxPlayer->GetState())
	{
		if(0 > m_pNxPlayer->Seek(mSec))
		{
			return false;
		}
	}
	return true;
}

bool PlayerVideoFrame::SetVideoMute(bool iStopRendering)
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGE("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}

	m_statusMutex.Lock();
	m_bStopRenderingFlag = iStopRendering;
	m_statusMutex.Unlock();

	NXLOGI("%s(), line: %d, SetVideoMute : %d -------\n", __FUNCTION__, __LINE__, m_bStopRenderingFlag);

	return true;
}

bool PlayerVideoFrame::VideoMuteStart()
{
	if(m_fSpeed > 1.0)
	{
		StopVideo();
		return true;
	}
	SetVideoMute(true);

	NXLOGI("==============drm dsp deInit++++++++++\n");
	m_pNxPlayer->DrmVideoMute(true);
	NXLOGI("==============drm dsp deInit----------\n");

	return true;
}

bool PlayerVideoFrame::VideoMuteStop()
{
	if(m_fSpeed > 1.0)
	{
		StopVideo();
		return true;
	}

	NXLOGI("==============drm dsp Init++++++++++\n");
	m_pNxPlayer->DrmVideoMute(false);
	NXLOGI("==============drm dsp Init----------\n");

	SetVideoMute(false);

	return true;
}

bool PlayerVideoFrame::VideoMute()
{
	if(GetVideoMuteStatus())
	{
		VideoMuteStop();
	}
	else
	{
		VideoMuteStart();
	}

	return true;
}

bool PlayerVideoFrame::GetVideoMuteStatus()
{
	return m_bStopRenderingFlag;
}

void PlayerVideoFrame::displayTouchEvent()
{
	if(false == m_bButtonHide)
	{
		m_bButtonHide = true;
		ui->progressBar->hide();
		ui->prevButton->hide();
		ui->playButton->hide();
		ui->pauseButton->hide();
		ui->nextButton->hide();
		ui->stopButton->hide();
		ui->playListButton->hide();
		ui->durationlabel->hide();
		ui->appNameLabel->hide();
		ui->speedButton->hide();
		m_pStatusBar->hide();
		NXLOGD("**************** MainWindow:: Hide \n ");
	}
	else
	{
		NXLOGD("**************** MainWindow:: Show \n ");
		ui->progressBar->show();
		ui->prevButton->show();
		ui->playButton->show();
		ui->pauseButton->show();
		ui->nextButton->show();
		ui->stopButton->show();
		ui->playListButton->show();
		ui->durationlabel->show();
		ui->appNameLabel->show();
		ui->speedButton->show();
		m_pStatusBar->show();
		m_bButtonHide = false;
	}
}

//
//		Play Util
//
void PlayerVideoFrame::getAspectRatio(int srcWidth, int srcHeight,
									  int scrWidth, int scrHeight,
									  int *pWidth,  int *pHeight)
{
	// Calculate Video Aspect Ratio
	int dspWidth = 0, dspHeight = 0;
	double xRatio = (double)scrWidth / (double)srcWidth;
	double yRatio = (double)scrHeight / (double)srcHeight;

	if( xRatio > yRatio )
	{
		dspWidth    = (int)((double)srcWidth * yRatio);
		dspHeight   = scrHeight;
	}
	else
	{
		dspWidth    = scrWidth;
		dspHeight   = (int)((double)srcHeight * xRatio);
	}

	*pWidth     = dspWidth;
	*pHeight    = dspHeight;
}

//
//		Play List Button
//
void PlayerVideoFrame::on_playListButton_released()
{
	if(m_bIsAudioFocus == false)
	{
		return;
	}

	if(NULL == m_pPlayListFrame)
	{
		m_pPlayListFrame = new PlayListVideoFrame(this);
		m_pPlayListFrame->RegisterRequestLauncherShow(m_pRequestLauncherShow);
		m_pPlayListFrame->RegisterRequestVolume(m_pRequestVolume);
		connect(m_pPlayListFrame, SIGNAL(signalPlayListAccept()), this, SLOT(slotPlayListFrameAccept()));
		connect(m_pPlayListFrame, SIGNAL(signalPlayListReject()), this, SLOT(slotPlayListFrameReject()));
	}
	m_pPlayListFrame->show();

	m_listMutex.Lock();
	m_pPlayListFrame->setList(&m_FileList);
	m_listMutex.Unlock();

	m_pPlayListFrame->setCurrentIndex(m_iCurFileListIdx);
}

void PlayerVideoFrame::slotPlayListFrameAccept()
{
	QApplication::postEvent(this, new AcceptEvent());
}

void PlayerVideoFrame::slotPlayListFrameReject()
{
	QApplication::postEvent(this, new RejectEvent());
}

bool PlayerVideoFrame::event(QEvent *event)
{
	switch ((int32_t)event->type())
	{
	case NX_CUSTOM_BASE_ACCEPT:
	{
		if(m_pPlayListFrame)
		{
			m_iCurFileListIdx = m_pPlayListFrame->getCurrentIndex();
			StopVideo();
			PlayVideo();

			delete m_pPlayListFrame;
			m_pPlayListFrame = NULL;
		}
		return true;
	}
	case NX_CUSTOM_BASE_REJECT:
	{
		if(m_pPlayListFrame)
		{
			delete m_pPlayListFrame;
			m_pPlayListFrame = NULL;
		}
		return true;
	}
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
	case E_NX_EVENT_STATUS_VOLUME:
	{
		NxStatusVolumeEvent *e = static_cast<NxStatusVolumeEvent *>(event);
		StatusVolumeEvent(e);
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

void PlayerVideoFrame::resizeEvent(QResizeEvent *)
{
	if ((width() != DEFAULT_DSP_WIDTH) || (height() != DEFAULT_DSP_HEIGHT))
	{
		SetupUI();
	}
}

void PlayerVideoFrame::SetupUI()
{
	ui->graphicsView->setGeometry(0,0,width(),height());

	float widthRatio = (float)width() / DEFAULT_DSP_WIDTH;
	float heightRatio = (float)height() / DEFAULT_DSP_HEIGHT;
	int rx, ry, rw, rh;

	rx = widthRatio * ui->progressBar->x();
	ry = heightRatio * ui->progressBar->y();
	rw = widthRatio * ui->progressBar->width();
	rh = heightRatio * ui->progressBar->height();
	ui->progressBar->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->prevButton->x();
	ry = heightRatio * ui->prevButton->y();
	rw = widthRatio * ui->prevButton->width();
	rh = heightRatio * ui->prevButton->height();
	ui->prevButton->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->playButton->x();
	ry = heightRatio * ui->playButton->y();
	rw = widthRatio * ui->playButton->width();
	rh = heightRatio * ui->playButton->height();
	ui->playButton->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->pauseButton->x();
	ry = heightRatio * ui->pauseButton->y();
	rw = widthRatio * ui->pauseButton->width();
	rh = heightRatio * ui->pauseButton->height();
	ui->pauseButton->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->nextButton->x();
	ry = heightRatio * ui->nextButton->y();
	rw = widthRatio * ui->nextButton->width();
	rh = heightRatio * ui->nextButton->height();
	ui->nextButton->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->stopButton->x();
	ry = heightRatio * ui->stopButton->y();
	rw = widthRatio * ui->stopButton->width();
	rh = heightRatio * ui->stopButton->height();
	ui->stopButton->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->playListButton->x();
	ry = heightRatio * ui->playListButton->y();
	rw = widthRatio * ui->playListButton->width();
	rh = heightRatio * ui->playListButton->height();
	ui->playListButton->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->durationlabel->x();
	ry = heightRatio * ui->durationlabel->y();
	rw = widthRatio * ui->durationlabel->width();
	rh = heightRatio * ui->durationlabel->height();
	ui->durationlabel->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->subTitleLabel->x();
	ry = heightRatio * ui->subTitleLabel->y();
	rw = widthRatio * ui->subTitleLabel->width();
	rh = heightRatio * ui->subTitleLabel->height();
	ui->subTitleLabel->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->subTitleLabel2->x();
	ry = heightRatio * ui->subTitleLabel2->y();
	rw = widthRatio * ui->subTitleLabel2->width();
	rh = heightRatio * ui->subTitleLabel2->height();
	ui->subTitleLabel2->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->speedButton->x();
	ry = heightRatio * ui->speedButton->y();
	rw = widthRatio * ui->speedButton->width();
	rh = heightRatio * ui->speedButton->height();
	ui->speedButton->setGeometry(rx, ry, rw, rh);
}

void PlayerVideoFrame::StatusHomeEvent(NxStatusHomeEvent *)
{
	if (m_pRequestLauncherShow)
	{
		bool bOk = false;
		m_pRequestOpacity(true);
		m_pRequestLauncherShow(&bOk);
		NXLOGI("[%s] REQUEST LAUNCHER SHOW <%s>", __FUNCTION__, bOk ? "OK" : "NG");
	}
}

void PlayerVideoFrame::StatusBackEvent(NxStatusBackEvent *)
{
	QApplication::postEvent(this, new NxTerminateEvent());
}

void PlayerVideoFrame::StatusVolumeEvent(NxStatusVolumeEvent *)
{
	if (m_pRequestVolume)
	{
		m_pRequestVolume();
	}
}

void PlayerVideoFrame::TerminateEvent(NxTerminateEvent *)
{
	if (m_pRequestTerminate)
	{
		if (m_pRequestOpacity)
		{
			m_pRequestOpacity(true);
		}
		m_pRequestTerminate();
	}
}

void PlayerVideoFrame::RegisterRequestTerminate(void (*cbFunc)(void))
{
	if (cbFunc)
	{
		m_pRequestTerminate = cbFunc;
	}
}

void PlayerVideoFrame::RegisterRequestVolume(void (*cbFunc)(void))
{
	if (cbFunc)
	{
		m_pRequestVolume = cbFunc;
	}
}


void PlayerVideoFrame::RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk))
{
	if (cbFunc)
	{
		m_pRequestLauncherShow = cbFunc;
	}
}

void PlayerVideoFrame::RegisterRequestOpacity(void (*cbFunc)(bool))
{
	if (cbFunc)
	{
		m_pRequestOpacity = cbFunc;
	}
}

//
// Subtitle Display Routine
//

void PlayerVideoFrame::subTitleDisplayUpdate()
{
	if(m_bSubThreadFlag)
	{
		if( (m_pNxPlayer) && (StoppedState != m_pNxPlayer->GetState()))
		{
			QString encResult;
			int idx;
			qint64 curPos = m_pNxPlayer->GetMediaPosition();
			for( idx = m_pNxPlayer->GetSubtitleIndex() ; idx <= m_pNxPlayer->GetSubtitleMaxIndex() ; idx++ )
			{
				if(m_pNxPlayer->GetSubtitleStartTime() < curPos)
				{
					char *pBuf = m_pNxPlayer->GetSubtitleText();
					encResult = m_pCodec->toUnicode(pBuf);

					//HTML
					//encResult = QString("%1").arg(m_pCodec->toUnicode(pBuf));	//&nbsp; not detected
					//encResult.replace( QString("<br>"), QString("\n")  );		//detected
					encResult.replace( QString("&nbsp;"), QString(" ")  );
					if(m_bButtonHide == false)
					{
						ui->subTitleLabel->setText(encResult);
						ui->subTitleLabel2->setText("");
					}
					else
					{
						ui->subTitleLabel->setText("");
						ui->subTitleLabel2->setText(encResult);
					}
				}else
				{
					break;
				}
				m_pNxPlayer->IncreaseSubtitleIndex();
			}
		}
	}
}


int PlayerVideoFrame::OpenSubTitle()
{
	QString path = m_FileList.GetList(m_iCurFileListIdx);
	int lastIndex = path.lastIndexOf(".");
	char tmpStr[1024]={0};
	if((lastIndex == 0))
	{
		return -1;  //this case means there is no file that has an extension..
	}
	strncpy(tmpStr, (const char*)path.toStdString().c_str(), lastIndex);
	QString pathPrefix(tmpStr);
	QString subtitlePath;

	subtitlePath = pathPrefix + ".smi";

	//call library method
	int openResult = m_pNxPlayer->OpenSubtitle( (char *)subtitlePath.toStdString().c_str() );

	if ( 1 == openResult )
	{
		// case of open succeed
		m_pCodec = QTextCodec::codecForName(m_pNxPlayer->GetBestSubtitleEncode());
		if (NULL == m_pCodec)
			m_pCodec = QTextCodec::codecForName("EUC-KR");
		return 0;
	}else if( -1 == openResult )
	{
		//smi open tried but failed while fopen (maybe smi file does not exist)
		//should try opening srt
		subtitlePath = pathPrefix + ".srt";
		if( 1 == m_pNxPlayer->OpenSubtitle( (char *)subtitlePath.toStdString().c_str() ) )
		{
			m_pCodec = QTextCodec::codecForName(m_pNxPlayer->GetBestSubtitleEncode());
			if (NULL == m_pCodec)
				m_pCodec = QTextCodec::codecForName("EUC-KR");
			return 0;
		}else
		{
			//smi and srt both cases are tried, but open failed
			return -1;
		}
	}else
	{
		NXLOGE("parser lib OpenResult : %d\n",openResult);
		//other err cases
		//should check later....
		return -1;
	}
	return -1;
}

void PlayerVideoFrame::PlaySubTitle()
{
	m_bSubThreadFlag = true;
}

void PlayerVideoFrame::StopSubTitle()
{
	if(m_bSubThreadFlag)
	{
		m_bSubThreadFlag = false;
	}

	m_pNxPlayer->CloseSubtitle();

	ui->subTitleLabel->setText("");
	ui->subTitleLabel2->setText("");
}

void PlayerVideoFrame::on_speedButton_released()
{
	if(StoppedState == m_pNxPlayer->GetState() || ReadyState == m_pNxPlayer->GetState() )
	{
		qDebug("Works when in play state.\n");
		ui->speedButton->setText("x 1");
		return;
	}

	if ( m_pNxPlayer->GetVideoSpeedSupport() < 0)
	{
		if(m_bNotSupportSpeed)
		{
			return;
		}
		else
		{
			m_pMessageFrame->show();
			m_pMessageLabel->setText("\n  Not Support Speed !!\n  -Support file(.mp4,.mkv,.avi)\n  -Support Codec(h264, mpeg4)\n");
			m_bNotSupportSpeed = true;
			return;
		}
	}

	m_fSpeed = m_fSpeed * 2;

	if(m_fSpeed > 16)
	{
		m_fSpeed = 1.0;
	}

	if(m_fSpeed ==1.0) ui->speedButton->setText("x 1");
	else if(m_fSpeed ==2.0) ui->speedButton->setText("x 2");
	else if(m_fSpeed ==4.0) ui->speedButton->setText("x 4");
	else if(m_fSpeed ==8.0) ui->speedButton->setText("x 8");
	else if(m_fSpeed ==16.0) ui->speedButton->setText("x 16");


	m_pNxPlayer->SetVideoSpeed( m_fSpeed  );

	if(m_fSpeed > 1.0)
	{
		QString style;
		style += "QProgressBar {";
		style += "  border: 2px solid grey;";
		style += "  border-radius: 5px;";
		style += "  background: grey;";
		style += "}";

		style += "QProgressBar::chunk {";
		style += "  background-color: rgb(37, 86, 201);";
		style += "width: 20px;";
		style += "}";

		ui->progressBar->setStyleSheet(style);
	}
	else
	{
		QString style;
		style += "QProgressBar {";
		style += "  border: 2px solid grey;";
		style += "  border-radius: 5px;";
		style += "  background: white;";
		style += "}";

		style += "QProgressBar::chunk {";
		style += "  background-color: rgb(37, 86, 201);";
		style += "width: 20px;";
		style += "}";

		ui->progressBar->setStyleSheet(style);
	}
}

void PlayerVideoFrame::slotOk()
{
	if(m_bNotSupportSpeed)
	{
		m_bNotSupportSpeed = false;
		m_pMessageFrame->hide();
	}
	else
	{
		ui->progressBar->setValue(0);
		UpdateDurationInfo(0, 0);
		StopVideo();
		m_pMessageFrame->hide();
	}}
