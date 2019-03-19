#include <QGraphicsDropShadowEffect>
#include <QGraphicsColorizeEffect>
#include <QThread>
#include <QTextCodec>
#include <QTimer>
#include <QDesktopWidget>

#include "PlayerAudioFrame.h"
#include "ui_PlayerAudioFrame.h"

#define LOG_TAG "[NxAudioPlayer|mainW]"
#include <NX_Log.h>

// ID3 Library
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <id3/tag.h>
#include <id3/field.h>
#include <id3/misc_support.h>
#pragma GCC diagnostic pop

//#define AUDIO_DEFAULT_DEVICE "plughw:0,0"
//#define AUDIO_HDMI_DEVICE    "plughw:0,3"

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
static void cbStatusHome( void *pObj )
{
	(void)pObj;
	PlayerAudioFrame *p = (PlayerAudioFrame *)pObj;
	QApplication::postEvent(p, new NxStatusHomeEvent());
}

static void cbStatusBack( void *pObj )
{
	PlayerAudioFrame *pW = (PlayerAudioFrame *)pObj;
	pW->SaveInfo();
	pW->StopAudio();
	pW->close();
	QApplication::postEvent(pW, new NxStatusBackEvent());
}

static void cbStatusVolume( void *pObj )
{
	PlayerAudioFrame *pW = (PlayerAudioFrame *)pObj;
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

	//if(type == audio)-->add
	if( !strcmp("audio", cFileType ))
	{
		(((PlayerAudioFrame*)pObj)->GetFileList())->AddItem( QString::fromUtf8(cFilePath) );
	}
	return 0;
}

PlayerAudioFrame::PlayerAudioFrame(QWidget *parent)
	: QFrame(parent)
	, m_pNxPlayer(NULL)
	, m_iDuration(0)
	, m_iVolumeValue(100)				//	Audio SW Volume should be set to 100.
	, m_bIsInitialized(false)
	, m_pStatusBar(NULL)
	, m_bSeekReady(false)
	, m_iCurFileListIdx (0)
	, m_bTurnOffFlag(false)
	, m_bTryFlag(false)
	, m_pPlayListFrame(NULL)
	, m_pRequestTerminate(NULL)
	, m_pRequestLauncherShow(NULL)
	, m_pRequestVolume(NULL)
	, m_bIsAudioFocus(true)
	, m_pMessageFrame(NULL)
	, m_pMessageLabel(NULL)
	, m_pMessageButton(NULL)
	, ui(new Ui::PlayerAudioFrame)
{
	ui->setupUi(this);

	const QRect screen = QApplication::desktop()->screenGeometry();
	if ((width() != screen.width()) || (height() != screen.height()))
	{
		setFixedSize(screen.width(), screen.height());
	}

	pthread_mutex_init(&m_listMutex, NULL);
	m_pNxPlayer = new CNX_AudioPlayer();
	UpdateFileList();

	m_pIConfig = GetConfigHandle();

	ui->progressBar->installEventFilter(this);

	//
	//	Initialize UI Controls
	//
	//	Nexell Status Bar
	m_pStatusBar = new CNX_StatusBar(this);
	m_pStatusBar->move( 0, 0 );
	m_pStatusBar->resize( this->size().width(), this->size().height() * 1 / 10 );
	m_pStatusBar->RegOnClickedHome( cbStatusHome );
	m_pStatusBar->RegOnClickedBack( cbStatusBack );
	m_pStatusBar->RegOnClickedVolume( cbStatusVolume );
	m_pStatusBar->SetTitleName( "Nexell Audio Player" );

	//	Run Position Update Timer
	connect(&m_PosUpdateTimer, SIGNAL(timeout()), this, SLOT(DoPositionUpdate()));
	connect(&m_PlayerSignals, SIGNAL(MediaPlayerCallback(int)), SLOT(MediaPlayerCallback(int)));
	m_PosUpdateTimer.start( 500 );

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
	if(0 > m_pIConfig->Open("/nexell/daudio/NxAudioPlayer/nxaudioplayer_config.xml"))
	{
		NXLOGE("[%s]nxaudioplayer_config.xml open err\n", __FUNCTION__);
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

PlayerAudioFrame::~PlayerAudioFrame()
{
	pthread_mutex_destroy(&m_listMutex);
	if(m_pNxPlayer)
	{
		NX_MediaStatus state = m_pNxPlayer->GetState();
		if( (PlayingState == state)||(PausedState == state) )
		{
			StopAudio();
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
int32_t PlayerAudioFrame::SaveInfo()
{
	if( NULL == m_pNxPlayer)
	{
		NXLOGI("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return -1;
	}

	if(0 > m_pIConfig->Open("/nexell/daudio/NxAudioPlayer/config.xml"))
	{
		NXLOGW("xml open err\n");
		QFile qFile;
		qFile.setFileName("/nexell/daudio/NxAudioPlayer/config.xml");
		if(qFile.remove())
		{
			NXLOGW("config.xml is removed because of open err\n");
			if(0 > m_pIConfig->Open("/nexell/daudio/NxAudioPlayer/config.xml"))
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
	pthread_mutex_lock(&m_listMutex);
	QString curPath = m_FileList.GetList(m_iCurFileListIdx);
	pthread_mutex_unlock(&m_listMutex);
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
			NXLOGD("current position is not valid  iCurPos : %lld is set to 0\n",iCurPos);
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

bool PlayerAudioFrame::SeekToPrev(int* iSavedPosition, int* iFileIdx)
{
	if( NULL == m_pNxPlayer)
	{
		NXLOGE("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}

	if(0 > m_pIConfig->Open("/nexell/daudio/NxAudioPlayer/config.xml"))
	{
		NXLOGW("xml open err\n");
		QFile qFile;
		qFile.setFileName("/nexell/daudio/NxAudioPlayer/config.xml");
		if(qFile.remove())
		{
			NXLOGW("config.xml is removed because of open err\n");
		}else
		{
			NXLOGW("Deleting config.xml is failed!\n");
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
	pthread_mutex_lock(&m_listMutex);
	if(0 < m_FileList.GetSize())
	{
		//media file list is valid
		//find pPrevPath in list

		int iIndex = m_FileList.GetPathIndex(QString::fromUtf8(pPrevPath));
		if(0 > iIndex)
		{
			NXLOGE("saved path does not exist in FileList\n");
			pthread_mutex_unlock(&m_listMutex);
			return false;
		}
		*iFileIdx = iIndex;
		pthread_mutex_unlock(&m_listMutex);
		return true;
	}else
	{
		NXLOGD("File List is not valid.. no media file or media scan is not done\n");
		NXLOGD("just try last path : %s\n\n",pPrevPath);
		m_bTryFlag = true;
		m_FileList.AddItem(QString::fromUtf8(pPrevPath));
		*iFileIdx = 0;
		pthread_mutex_unlock(&m_listMutex);
		return true;
	}

	return false;
}

//
//	Storage Event
void PlayerAudioFrame::StorageRemoved()
{
	m_bTurnOffFlag = true;

	if(NULL != m_pNxPlayer)
	{
		SaveInfo();
		StopAudio();
	}else
	{
		NXLOGD("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
	}

	if(NULL != m_pPlayListFrame)
	{
		m_pPlayListFrame->close();
	}

	//this->close();
	QApplication::postEvent(this, new NxTerminateEvent());
}

void PlayerAudioFrame::StorageScanDone()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGD("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return;
	}
	pthread_mutex_lock(&m_listMutex);
	bool bPlayFlag = false;
	if(0 == m_FileList.GetSize())
	{
		//player is obviously not playing..
		UpdateFileList();
		if(0 != m_FileList.GetSize())
		{
			bPlayFlag = true;
		}
	}else
	{
		//player could be playing some file...
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
	pthread_mutex_unlock(&m_listMutex);

	if(NULL != m_pPlayListFrame)
	{
		pthread_mutex_lock(&m_listMutex);
		m_pPlayListFrame->setList(&m_FileList);
		pthread_mutex_unlock(&m_listMutex);
	}

	if(bPlayFlag)
	{
		PlaySeek();
	}
}

void PlayerAudioFrame::UpdateFileList()
{
	//	read data base that Media Scaning made.
	char szPath[256];
	snprintf( szPath, sizeof(szPath), "%s/%s", NX_MEDIA_DATABASE_PATH, NX_MEDIA_DATABASE_NAME );
	int ret = NX_SQLiteGetData( szPath, NX_MEDIA_DATABASE_TABLE, cbSqliteRowCallback, (void*)this);
	NXLOGD("<<< Total file list = %d, ret(%d)\n", m_FileList.GetSize(), ret);
}

CNX_FileList *PlayerAudioFrame::GetFileList()
{
	return &m_FileList;
}

bool PlayerAudioFrame::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == ui->progressBar)
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

bool PlayerAudioFrame::PlayAudio()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}

	if(PlayingState == m_pNxPlayer->GetState())
	{
		NXLOGD("already playing\n");
		return true;
	}

	if(PausedState == m_pNxPlayer->GetState())
	{
		m_pNxPlayer->Play();
		return true;
	}

	//	is paused : just plaing
	if(StoppedState == m_pNxPlayer->GetState())
	{
		pthread_mutex_lock(&m_listMutex);
		if( 0 < m_FileList.GetSize() )
		{
			int iResult = -1;
			int iTryCount = 0;
			int iMaxCount = m_FileList.GetSize();
			while(0 > iResult)
			{
				iResult = m_pNxPlayer->InitMediaPlayer( PlayerAudioFrame::PlayerCallbackStub,
														this,
														m_FileList.GetList(m_iCurFileListIdx).toStdString().c_str(),
														MP_TRACK_AUDIO,
														m_audioDeviceName,
														NULL
														);
				iTryCount++;
				if(0 == iResult)
				{
					NXLOGD("\n\n********************media init done!\n");
					m_bIsInitialized = true;

					if( 0 > m_pNxPlayer->Play() )
					{
						NXLOGE("NX_MPPlay() failed !!!");
						iResult = -1; //retry with next file..
					}else
					{
						//play succeed
						//                        SetVolume(m_iVolumeValue);
						m_iDuration = m_pNxPlayer->GetMediaDuration();
						ui->progressBar->setMaximum( m_iDuration/1000 );	//	Max Seconds

						//	Update Applications
						UpdateAlbumInfo();
						NXLOGD("********************media play done!\n\n\n");
						pthread_mutex_unlock(&m_listMutex);
						return true;
					}
				}

				if(m_bTryFlag)
				{
					//This case is for playing back to last file
					//When there is no available file list but last played file path exists in config.xml,
					//videoplayer tries playing path saved in config.xml .
					//If trying playing is failed, videoplayer should stop trying next file.
					NXLOGD( "Have no available contents!!\n" );
					m_bTryFlag = false;
					m_FileList.ClearList();
					pthread_mutex_unlock(&m_listMutex);
					return false;
				}

				if( iTryCount == iMaxCount )
				{
					//all list is tried, but nothing succeed.
					NXLOGI( "Have no available contents!!\n" );
					pthread_mutex_unlock(&m_listMutex);
					return false;
				}

				NXLOGD("MediaPlayer Initialization fail.... retry with next file\n");
				if(0 != m_FileList.GetSize())
				{
					m_iCurFileListIdx = (m_iCurFileListIdx+1) % m_FileList.GetSize();
				}
				CloseAudio();
			}
		}else
		{
			NXLOGD( "Have no available contents!!" );
			pthread_mutex_unlock(&m_listMutex);
			return false;
		}
	}
	return true;
}

bool PlayerAudioFrame::StopAudio()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}
	m_pNxPlayer->Stop();
	CloseAudio();

	return true;
}

bool PlayerAudioFrame::PauseAudio()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}

	if(PlayingState != m_pNxPlayer->GetState())
	{
		return false;
	}
	if(0 > m_pNxPlayer->Pause())
		return false;

	return true;
}

bool PlayerAudioFrame::SeekAudio( int32_t mSec )
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
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

bool PlayerAudioFrame::CloseAudio()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}

	m_bIsInitialized = false;
	if(0 > m_pNxPlayer->CloseHandle())
	{
		NXLOGE("%s(), line: %d, CloseHandle failed \n", __FUNCTION__, __LINE__);
		return false;
	}

	return true;
}

bool PlayerAudioFrame::SetAudioVolume( int32_t volume )
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


bool PlayerAudioFrame::PlayNextAudio()
{
	StopAudio();

	//	find next index
	if(0 != m_FileList.GetSize())
	{
		m_iCurFileListIdx = (m_iCurFileListIdx+1) % m_FileList.GetSize();
	}
	return PlayAudio();
}

bool PlayerAudioFrame::PlayPreviousAudio()
{
	StopAudio();

	//	Find previous index
	if(0 != m_FileList.GetSize())
	{
		m_iCurFileListIdx --;
		if( 0 > m_iCurFileListIdx )
			m_iCurFileListIdx = m_FileList.GetSize() -1;
	}
	return PlayAudio();
}

void PlayerAudioFrame::PlaySeek()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return;
	}

	bool seekflag = false;
	int iSavedPosition = 0;
	seekflag = SeekToPrev(&iSavedPosition, &m_iCurFileListIdx);

	PlayAudio();

	if(seekflag)
	{
		//seek audio
		SeekAudio(iSavedPosition);
	}
}

int PlayerAudioFrame::GetFileListSize()
{
	return m_FileList.GetSize();
}

void PlayerAudioFrame::PlayerCallback(unsigned int eventType, unsigned int eventData, unsigned int dataSize)
{
	(void) eventData;
	(void) dataSize;
	m_PlayerSignals.MediaPlayerCallback( eventType );
}

void PlayerAudioFrame::MediaPlayerCallback( int eventType )
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
		NXLOGD("******** EndOfMedia !!!\n");
		PlayNextAudio();
		break;
	}
	case MP_MSG_DEMUX_ERR:
	{
		NXLOGD("******** MP_MSG_DEMUX_ERR !!!\n");
		// message
		m_pMessageFrame->show();
		m_pMessageLabel->setText("DEMUX_ERR");

		ui->progressBar->setValue(0);
		UpdateDurationInfo( 0, 0 );
		StopAudio();
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
		StopAudio();
		break;
	}

	default:
		break;
	}
}

void PlayerAudioFrame::DoPositionUpdate()
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

void PlayerAudioFrame::UpdateDurationInfo(int64_t position, int64_t duration)
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
	//NXLOGV(">> %s() Out %s", __func__, tStr.toStdString().c_str());
}


////////////////////////////////////////////////////////////////////
//
//		Update Player Progress Bar & Volume Progress Bar
//
////////////////////////////////////////////////////////////////////
void PlayerAudioFrame::updateProgressBar(QMouseEvent *event, bool bReleased)
{
	if( bReleased )
	{
		//	 Do Seek
		if( m_bSeekReady )
		{
			if( StoppedState != m_pNxPlayer->GetState() )
			{
				double ratio = (double)event->x()/ui->progressBar->width();
				qint64 position = ratio * m_iDuration;
				SeekAudio(position);
				NXLOGV("audioPlayer Seek to Position = %lld", position);
			}
			m_PosUpdateTimer.start(500);
		}
		m_bSeekReady = false;
	}
	else
	{
		m_bSeekReady = true;
	}
}

void PlayerAudioFrame::setAudioFocus(bool bAudioFocus)
{
	m_bIsAudioFocus = bAudioFocus;
}

void PlayerAudioFrame::on_prevButton_released()
{
	if(m_bIsAudioFocus == false)
	{
		return;
	}

	PlayPreviousAudio();
}

void PlayerAudioFrame::on_playButton_released()
{
	if(m_bIsAudioFocus == false)
	{
		return;
	}

	PlayAudio();
}

void PlayerAudioFrame::on_pauseButton_released()
{
	if(m_bIsAudioFocus == false)
	{
		return;
	}

	PauseAudio();
}

void PlayerAudioFrame::on_nextButton_released()
{
	if(m_bIsAudioFocus == false)
	{
		return;
	}

	PlayNextAudio();
}

void PlayerAudioFrame::on_stopButton_released()
{
	if(m_bIsAudioFocus == false)
	{
		return;
	}

	StopAudio();
}

//
//		Play List Button
//
void PlayerAudioFrame::on_playListButton_released()
{
	if(m_bIsAudioFocus == false)
	{
		return;
	}

	if(NULL == m_pPlayListFrame)
	{
		m_pPlayListFrame = new PlayListAudioFrame(this);
		m_pPlayListFrame->RegisterRequestLauncherShow(m_pRequestLauncherShow);
		m_pPlayListFrame->RegisterRequestVolume(m_pRequestVolume);
		connect(m_pPlayListFrame, SIGNAL(signalPlayListAccept()), this, SLOT(slotPlayListFrameAccept()));
		connect(m_pPlayListFrame, SIGNAL(signalPlayListReject()), this, SLOT(slotPlayListFrameReject()));
	}
	m_pPlayListFrame->show();

	pthread_mutex_lock(&m_listMutex);
	m_pPlayListFrame->setList(&m_FileList);
	pthread_mutex_unlock(&m_listMutex);

	m_pPlayListFrame->setCurrentIndex(m_iCurFileListIdx);
}

void PlayerAudioFrame::slotPlayListFrameAccept()
{
	QApplication::postEvent(this, new AcceptEvent());
}

void PlayerAudioFrame::slotPlayListFrameReject()
{
	QApplication::postEvent(this, new RejectEvent());
}

bool PlayerAudioFrame::event(QEvent *event)
{
	switch ((int32_t)event->type())
	{
	case NX_CUSTOM_BASE_ACCEPT:
	{
		if(m_pPlayListFrame)
		{
			m_iCurFileListIdx = m_pPlayListFrame->getCurrentIndex();
			StopAudio();
			PlayAudio();

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

void PlayerAudioFrame::resizeEvent(QResizeEvent *)
{
	if ((width() != DEFAULT_DSP_WIDTH) || (height() != DEFAULT_DSP_HEIGHT))
	{
		SetupUI();
	}
}

void PlayerAudioFrame::SetupUI()
{
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

	//
	rx = widthRatio * ui->albumArtView->x();
	ry = heightRatio * ui->albumArtView->y();
	rw = widthRatio * ui->albumArtView->width();
	rh = heightRatio * ui->albumArtView->height();
	ui->albumArtView->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->labelAlbum->x();
	ry = heightRatio * ui->labelAlbum->y();
	rw = widthRatio * ui->labelAlbum->width();
	rh = heightRatio * ui->labelAlbum->height();
	ui->labelAlbum->setGeometry(rx, ry, rw, rh);


	rx = widthRatio * ui->labelArtist->x();
	ry = heightRatio * ui->labelArtist->y();
	rw = widthRatio * ui->labelArtist->width();
	rh = heightRatio * ui->labelArtist->height();
	ui->labelArtist->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->labelGenre->x();
	ry = heightRatio * ui->labelGenre->y();
	rw = widthRatio * ui->labelGenre->width();
	rh = heightRatio * ui->labelGenre->height();
	ui->labelGenre->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->labelTitle->x();
	ry = heightRatio * ui->labelTitle->y();
	rw = widthRatio * ui->labelTitle->width();
	rh = heightRatio * ui->labelTitle->height();
	ui->labelTitle->setGeometry(rx, ry, rw, rh);

	rx = widthRatio * ui->labelTrackNumber->x();
	ry = heightRatio * ui->labelTrackNumber->y();
	rw = widthRatio * ui->labelTrackNumber->width();
	rh = heightRatio * ui->labelTrackNumber->height();
	ui->labelTrackNumber->setGeometry(rx, ry, rw, rh);
}

void PlayerAudioFrame::StatusHomeEvent(NxStatusHomeEvent *)
{
	if (m_pRequestLauncherShow)
	{
		bool bOk = false;
		m_pRequestLauncherShow(&bOk);
		NXLOGI("[%s] REQUEST LAUNCHER SHOW <%s>", __FUNCTION__, bOk ? "OK" : "NG");
	}
}

void PlayerAudioFrame::StatusBackEvent(NxStatusBackEvent *)
{
	QApplication::postEvent(this, new NxTerminateEvent());
}

void PlayerAudioFrame::StatusVolumeEvent(NxStatusVolumeEvent *)
{
	if (m_pRequestVolume)
	{
		m_pRequestVolume();
	}
}

void PlayerAudioFrame::TerminateEvent(NxTerminateEvent *)
{
	if (m_pRequestTerminate)
	{
		m_pRequestTerminate();
	}
}

void PlayerAudioFrame::RegisterRequestTerminate(void (*cbFunc)(void))
{
	if (cbFunc)
	{
		m_pRequestTerminate = cbFunc;
	}
}

void PlayerAudioFrame::RegisterRequestVolume(void (*cbFunc)(void))
{
	if (cbFunc)
	{
		m_pRequestVolume = cbFunc;
	}
}

void PlayerAudioFrame::RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk))
{
	if (cbFunc)
	{
		m_pRequestLauncherShow = cbFunc;
	}
}


//
//	Update Album Information using ID3Tag
//

void PlayerAudioFrame::UpdateAlbumInfo()
{
	if(!isActiveWindow())
	{
		return;
	}

	QTextCodec * codec = QTextCodec::codecForName("eucKR");
	QString fileName = m_FileList.GetList(m_iCurFileListIdx);

	//	ID3 Tag Update using libid3 library
	size_t num;
	char *str;
	ID3_Tag id3Tag;

	//	Get Album General Information From ID3V1 Parser
	id3Tag.Clear();
	id3Tag.Link(fileName.toStdString().c_str(), ID3TT_ID3);

	//	Album
	str = ID3_GetAlbum( &id3Tag );
	ui->labelAlbum->setText("Album : "  + codec->toUnicode(str) );
	delete []str;

	//	Artist
	str = ID3_GetArtist( &id3Tag );
	ui->labelArtist->setText("Artist : " + codec->toUnicode(str) );
	delete []str;

	//	Title
	str = ID3_GetTitle( &id3Tag );
	ui->labelTitle->setText("Title : "  + codec->toUnicode(str) );
	delete []str;

	//	Track Number
	num = ID3_GetTrackNum( &id3Tag );
	ui->labelTrackNumber->setText("Artist : " + codec->toUnicode(QString::number(num).toStdString().c_str()));
	//	Genre : ID3 Version 1 Information
	num = ID3_GetGenreNum( &id3Tag );
	if( 0 < num && num < ID3_NR_OF_V1_GENRES )
	{
		ui->labelGenre->setText("Genre : "  + codec->toUnicode(ID3_v1_genre_description[num]) );
	}
	else
	{
		ui->labelGenre->setText("Genre : unknown");
	}

	//	Get Album Cover from ID3V2 Parser
	id3Tag.Clear();
	id3Tag.Link(fileName.toStdString().c_str(), ID3TT_ALL);

	if( ID3_HasPicture( &id3Tag ) )
	{
		//NXLOGD("Has Picture!!!!\n");
		ID3_GetPictureData(&id3Tag, "./temp.jpg");
		QPixmap pix("./temp.jpg");
		QPixmap scaledPix = pix.scaledToHeight(ui->albumArtView->height());
		m_AlbumArt.setPixmap(pix.scaledToHeight(ui->albumArtView->height()));
		ui->albumArtView->setScene(&m_Scene);
		m_Scene.addItem(&m_AlbumArt);
	}
	else
	{
		QPixmap pix(":/default.jpg");
		QPixmap scaledPix = pix.scaledToHeight(ui->albumArtView->height());
		m_AlbumArt.setPixmap(pix.scaledToHeight(ui->albumArtView->height()));
		ui->albumArtView->setScene(&m_Scene);
		m_Scene.addItem(&m_AlbumArt);
	}
}

void PlayerAudioFrame::slotOk()
{
	ui->progressBar->setValue(0);
	UpdateDurationInfo(0, 0);
	StopAudio();
	m_pMessageFrame->hide();
}
