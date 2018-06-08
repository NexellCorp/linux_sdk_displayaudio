#include <QGraphicsDropShadowEffect>
#include <QGraphicsColorizeEffect>
#include <QThread>
#include <QTextCodec>
#include <QTimer>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#define LOG_TAG "[NxAudioPlayer|mainW]"
#include <NX_Log.h>

// ID3 Library
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <id3/tag.h>
#include <id3/field.h>
#include <id3/misc_support.h>
#pragma GCC diagnostic pop

#define AUDIO_DEFAULT_DEVICE "plughw:0,0"
#define AUDIO_HDMI_DEVICE    "plughw:0,3"


static void cbStatusHome( void *pObj )
{
	(void)pObj;
	int32_t iRet = NX_RequestCommand( NX_REQUEST_LAUNCHER_SHOW );
	if( NX_REPLY_DONE != iRet)
	{
		NXLOGE( "Fail, NX_RequestCommand(). ( cmd: 0x%04X )\n", NX_REQUEST_LAUNCHER_SHOW );
	}
}

static void cbStatusBack( void *pObj )
{
	MainWindow *pW = (MainWindow *)pObj;
	pW->SaveInfo();
	pW->Stop();
	pW->CloseAudio();
	pW->close();
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
		(((MainWindow*)pObj)->GetFileList())->AddItem( QString::fromUtf8(cFilePath) );
	}
	return 0;
}


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_pNxPlayer(NULL)
	, m_iDuration(0)
	, m_iVolumeValue(100)				//	Audio SW Volume should be set to 100.
	, m_bIsInitialized(false)
	, m_pStatusBar(NULL)
	, m_bSeekReady(false)
	, m_bVolumeCtrlReady (false)
	, ui(new Ui::MainWindow)
	, m_iCurFileListIdx (0)
	, m_bTurnOffFlag(false)
	, m_bFocusTransientLossFlag(false)
	, m_iPrevState(StoppedState)
	, m_bTryFlag(false)
	, m_pPlayListWindow(NULL)
	, m_bRequestFocus(false)
{
	pthread_mutex_init(&m_listMutex, NULL);
	m_pNxPlayer = new CNX_MoviePlayer();
	UpdateFileList();

	m_pIConfig = GetConfigHandle();
	installEventFilter( this );
	NX_PacpClientStart(this);
	ui->setupUi(this);
	//
	//	Initialize UI Controls
	//
	//	Nexell Status Bar
	m_pStatusBar = new CNX_StatusBar(this);
	m_pStatusBar->move( 0, 0 );
	m_pStatusBar->resize( this->size().width(), this->size().height() * 1 / 10 );
	m_pStatusBar->RegOnClickedHome( cbStatusHome );
	m_pStatusBar->RegOnClickedBack( cbStatusBack );
	m_pStatusBar->SetTitleName( "Nexell Audio Player" );
	m_pStatusBar->SetVolume(m_iVolumeValue);

	//	Initialize Volume Control & Player's Volume
	ui->volumeProgressBar->setValue(m_iVolumeValue);
	ui->volumelabel->setText(QString("Vol: %1").arg(m_iVolumeValue));

	//	Run Position Update Timer
	connect(&m_PosUpdateTimer, SIGNAL(timeout()), this, SLOT(DoPositionUpdate()));
	connect(&m_PlayerSignals, SIGNAL(MediaPlayerCallback(int)), SLOT(MediaPlayerCallback(int)));
	m_PosUpdateTimer.start( 500 );
}

MainWindow::~MainWindow()
{
	pthread_mutex_destroy(&m_listMutex);
	removeEventFilter( this );
	NX_PacpClientStop();
	if(m_pNxPlayer)
	{
		delete m_pNxPlayer;
		m_pNxPlayer = NULL;
	}
	if(m_pPlayListWindow)
	{
		delete m_pPlayListWindow;
		m_pPlayListWindow = NULL;
	}
	delete ui;
}

//
//	for focus, close control
void MainWindow::SetTurnOffFlag(bool bFlag)
{
	m_bTurnOffFlag = bFlag;
}

void MainWindow::SetFocusTransientLossFlag(bool bFlag)
{
	m_bFocusTransientLossFlag = bFlag;

	if(NULL == m_pNxPlayer)
	{
		NXLOGI("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return;
	}
	if(PlayingState == m_pNxPlayer->GetState())
	{
		m_iPrevState = PlayingState;
	}
}

bool MainWindow::GetFocusTransientLossFlag()
{
	return m_bFocusTransientLossFlag;
}

void MainWindow::RestoreState()
{
	m_bFocusTransientLossFlag = false;
	if(PlayingState == m_iPrevState)
	{
		PlaySeek();
		m_iPrevState = StoppedState;
	}
}

//
//	xml (save previous state)
int MainWindow::SaveInfo()
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

bool MainWindow::SeekToPrev(int* iSavedPosition, int* iFileIdx)
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
void MainWindow::StorageRemoved()
{
	m_bTurnOffFlag = true;

	if(NULL != m_pNxPlayer)
	{
		SaveInfo();
		Stop();
		CloseAudio();
	}else
	{
		NXLOGD("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
	}

	if(NULL != m_pPlayListWindow)
	{
		m_pPlayListWindow->close();
	}

	this->close();
}

void MainWindow::StorageScanDone()
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

	if(NULL != m_pPlayListWindow)
	{
		pthread_mutex_lock(&m_listMutex);
		m_pPlayListWindow->setList(&m_FileList);
		pthread_mutex_unlock(&m_listMutex);
	}

	if(bPlayFlag)
	{
		PlaySeek();
	}
}

void MainWindow::UpdateFileList()
{
	//	read data base that Media Scaning made.
	char szPath[256];
	snprintf( szPath, sizeof(szPath), "%s/%s", NX_MEDIA_DATABASE_PATH, NX_MEDIA_DATABASE_NAME );
	NX_SQLiteGetData( szPath, NX_MEDIA_DATABASE_TABLE, cbSqliteRowCallback, (void*)this);
	NXLOGD("<<< Total file list = %d\n", m_FileList.GetSize());
}

CNX_FileList *MainWindow::GetFileList()
{
	return &m_FileList;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
	//
	//  Custom Event.
	//
	if( event->type() == NX_QT_CUSTOM_EVENT_TYPE )
	{
		NxEvent *pEvent = reinterpret_cast<NxEvent*>(event);
		switch( pEvent->m_iEventType )
		{
		case NX_REQUEST_PROCESS_SHOW:
			NX_PacpClientRequestRaise();
			break;

		case NX_REQUEST_FOCUS_VIDEO:
		case NX_REQUEST_FOCUS_VIDEO_TRANSIENT:
			if(NULL != m_pPlayListWindow)
			{
				m_pPlayListWindow->close();
			}
			break;

		case NX_REQUEST_FOCUS_AUDIO:
			m_bRequestFocus = true;
			// this->close();
			break;

		default:
			break;
		}
	}

	if( (event->type() == QEvent::ActivationChange) && isActiveWindow() )
	{
		NX_ReplyDone();
		if(0 != m_FileList.GetSize())
		{
			UpdateAlbumInfo();
		}
	}

	if( (event->type() == QEvent::ActivationChange) && !isActiveWindow() )
	{
		if( m_bRequestFocus )
		{
			this->close();
		}
	}

	return QMainWindow::eventFilter(watched, event);
}

void MainWindow::showEvent(QShowEvent* event)
{
	Q_UNUSED( event );

	PlaySeek();
	if(!isHidden())
	{
		return;
	}
	//Initialization of mediaPlayer must be done after QMainWindow::show() is done
	int32_t iRet = NX_RequestCommand( NX_REQUEST_PROCESS_SHOW );
	if( NX_REPLY_DONE != iRet )
	{
		NXLOGE( "Fail, NX_RequestCommand(). ( NX_REQUEST_PROCESS_SHOW iRet: %d )\n", iRet );
	}

	QMainWindow::show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	Q_UNUSED( event );

	int32_t iRet = NX_RequestCommand( NX_REQUEST_FOCUS_AUDIO_LOSS );
	if( NX_REPLY_DONE != iRet )
	{
		NXLOGE( "Fail, NX_RequestCommand(). ( NX_REQUEST_FOCUS_AUDIO_LOSS iRet: %d )\n", iRet );
	}

	if(isActiveWindow())
	{
		iRet = NX_RequestCommand( NX_REQUEST_FOCUS_VIDEO_LOSS );
		if( NX_REPLY_DONE != iRet )
		{
			NXLOGE( "Fail, NX_RequestCommand(). ( NX_REQUEST_FOCUS_VIDEO_LOSS iRet: %d )\n", iRet );
		}
	}

	iRet = NX_RequestCommand(NX_REQUEST_PROCESS_REMOVE);
	if( NX_REPLY_DONE != iRet )
	{
		NXLOGE( "Fail, NX_RequestCommand(). ( NX_REQUEST_PROCESS_REMOVE iRet: %d )\n", iRet );
	}

	QMainWindow::close();
}

bool MainWindow::Play()
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
				iResult = m_pNxPlayer->InitMediaPlayer( MainWindow::PlayerCallbackStub,
														this,
														m_FileList.GetList(m_iCurFileListIdx).toStdString().c_str(),
														MP_TRACK_AUDIO,
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
						SetVolume(m_iVolumeValue);
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

bool MainWindow::Stop()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return false;
	}
	m_pNxPlayer->Stop();
	return true;
}

bool MainWindow::Pause()
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

bool MainWindow::Seek( int32_t mSec )
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

bool MainWindow::CloseAudio()
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

bool MainWindow::SetVolume( int32_t volume )
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


bool MainWindow::PlayNextMusic()
{
	Stop();
	CloseAudio();

	//	find next index
	if(0 != m_FileList.GetSize())
	{
		m_iCurFileListIdx = (m_iCurFileListIdx+1) % m_FileList.GetSize();
	}
	return Play();
}

bool MainWindow::PlayPreviousMusic()
{
	Stop();
	CloseAudio();

	//	Find previous index
	if(0 != m_FileList.GetSize())
	{
		m_iCurFileListIdx --;
		if( 0 > m_iCurFileListIdx )
			m_iCurFileListIdx = m_FileList.GetSize() -1;
	}
	return Play();
}

void MainWindow::PlaySeek()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
		return;
	}

	bool seekflag = false;
	int iSavedPosition = 0;
	seekflag = SeekToPrev(&iSavedPosition, &m_iCurFileListIdx);

	Play();

	if(seekflag)
	{
		//seek audio
		Seek(iSavedPosition);
	}
}

void MainWindow::PlayerCallback(unsigned int eventType, unsigned int eventData, unsigned int dataSize)
{
	(void) eventData;
	(void) dataSize;
	m_PlayerSignals.MediaPlayerCallback( eventType );
}

void MainWindow::MediaPlayerCallback( int eventType )
{
	if(m_bTurnOffFlag)
	{
		NXLOGW("%s  , line : %d close app soon.. bypass \n", __FUNCTION__, __LINE__);
		return;
	}
	switch (eventType)
	{
	case MP_MSG_EOS:
		NXLOGD("******** EndOfMedia !!!\n");
		PlayNextMusic();
		break;

	case MP_MSG_DEMUX_ERR:
		NXLOGD("******** MP_MSG_DEMUX_ERR !!!\n");
		PlayNextMusic();
		break;

	case MP_MSG_ERR_OPEN_AUDIO_DEVICE:
		NXLOGE("******** MP_MSG_ERR_OPEN_AUDIO_DEVICE\n");
		PlayNextMusic();
		break;
	}
}

void MainWindow::DoPositionUpdate()
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

void MainWindow::UpdateDurationInfo(int64_t position, int64_t duration)
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
void MainWindow::updateProgressBar(QMouseEvent *event, bool bReleased)
{
	int x = event->x();
	int y = event->y();

	//  Progress Bar Update
	QPoint pos = ui->progressBar->pos();
	int width = ui->progressBar->width();
	int height = ui->progressBar->height();
	int minX = pos.rx();
	int maxX = pos.rx() + width;
	int minY = pos.ry();
	int maxY = pos.ry() + height;

	if( bReleased )
	{
		if( (minX<=x && x<=maxX) && (minY<=y && y<=maxY) )
		{
			//	 Do Seek
			if( m_bSeekReady )
			{
				if( StoppedState != m_pNxPlayer->GetState() )
				{
					double ratio = (double)(x-minX)/(double)width;
					qint64 position = ratio * m_iDuration;
					Seek(position);
					//NXLOGV("audioPlayer Seek to Position = %lld", position);
				}
				m_PosUpdateTimer.start(500);
			}
		}
		m_bSeekReady = false;
	}
	else
	{
		if( (minX<=x && x<=maxX) && (minY<=y && y<=maxY) )
		{
			m_bSeekReady = true;
		}
		else
		{
			m_bSeekReady = false;
		}
	}
}

void MainWindow::updateVolumeBar(QMouseEvent *event, bool bReleased)
{
	int x = event->x();
	int y = event->y();

	//  Progress Bar Update
	QPoint pos = ui->volumeProgressBar->pos();
	int width = ui->volumeProgressBar->width();
	int height = ui->volumeProgressBar->height();
	int minX = pos.rx();
	int maxX = pos.rx() + width;
	int minY = pos.ry();
	int maxY = pos.ry() + height;

	if( bReleased )
	{
		if( (minX<=x && x<=maxX) && (minY<=y && y<=maxY) )
		{
			//	 Change Volume
			if( m_bVolumeCtrlReady )
			{
				double ratio = (double)(maxY-y)/(double)height;
				qint64 position = ratio * VOLUME_MAX;
				m_iVolumeValue = position;
				ui->volumeProgressBar->setValue(m_iVolumeValue);
				ui->volumelabel->setText(QString("%1").arg(m_iVolumeValue));
				SetVolume(m_iVolumeValue);
				NXLOGD("audioPlayer Change Volume Value = %lld", position);
			}
		}
		m_bVolumeCtrlReady = false;
	}
	else
	{
		if( (minX<=x && x<=maxX) && (minY<=y && y<=maxY) )
		{
			m_bVolumeCtrlReady = true;
		}
		else
		{
			m_bVolumeCtrlReady = false;
		}
	}
}


void MainWindow::mousePressEvent(QMouseEvent *event)
{
	updateProgressBar(event, false);
	updateVolumeBar(event, false);
}


void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
	updateProgressBar(event, true);
	updateVolumeBar(event, true);
}

//
//
//		Player Control Button Events
//
//
void MainWindow::on_prevButton_released()
{
	PlayPreviousMusic();
}

void MainWindow::on_playButton_released()
{
	Play();
}

void MainWindow::on_pauseButton_released()
{
	Pause();
}

void MainWindow::on_nextButton_released()
{
	PlayNextMusic();
}

void MainWindow::on_stopButton_released()
{
	Stop();
}


//
//		Play List Button
//
void MainWindow::on_playListButton_released()
{
	if(NULL == m_pPlayListWindow)
	{
		m_pPlayListWindow = new PlayListWindow(this);
		connect(m_pPlayListWindow, SIGNAL(accepted()), this, SLOT(slotPlayListWindowAccepted()));
		connect(m_pPlayListWindow, SIGNAL(rejected()), this, SLOT(slotPlayListWindowRejected()));
	}
	m_pPlayListWindow->show();

	pthread_mutex_lock(&m_listMutex);
	m_pPlayListWindow->setList(&m_FileList);
	pthread_mutex_unlock(&m_listMutex);

	m_pPlayListWindow->setCurrentIndex(m_iCurFileListIdx);
}

void MainWindow::slotPlayListWindowAccepted()
{
	m_iCurFileListIdx = m_pPlayListWindow->getCurrentIndex();
	Stop();
	Play();
	m_pPlayListWindow = NULL;
}

void MainWindow::slotPlayListWindowRejected()
{
	m_pPlayListWindow = NULL;
}

//
//	Update Album Information using ID3Tag
//

void MainWindow::UpdateAlbumInfo()
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
	id3Tag.Link(fileName.toStdString().c_str(), ID3TT_ID3V1 | ID3TT_LYRICS3V2 | ID3TT_MUSICMATCH);
	//id3Tag.Link(fileName.toStdString().c_str(), ID3TT_ID3V1);

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
