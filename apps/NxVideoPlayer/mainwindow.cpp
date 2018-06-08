#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qtglvideowindow.h"
#include <QTextCodec>

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

#define LOG_TAG "[VideoPlayer|MainW]"
#include <NX_Log.h>


////////////////////////////////////////////////////////////////////////////////
//
//	Event Callback
//
static	CallBackSignal mediaStateCb;
static	MainWindow *pMainWindow = NULL;

//CallBack Eos, Error
static void cbEventCallback( void */*privateDesc*/, unsigned int EventType, unsigned int /*EventData*/, unsigned int /*param*/ )
{
	mediaStateCb.statusChanged(EventType);
}

//CallBack Qt
static void cbUpdateRender( void *pImg )
{
	(void) pImg;
	pMainWindow->ImageUpdate(pImg);
}

static void cbStatusHome( void *pObj )
{
	(void)pObj;
	int32_t iRet = NX_RequestCommand( NX_REQUEST_LAUNCHER_SHOW );
	if( NX_REPLY_DONE != iRet)
	{
		NXLOGE( "Fail, NX_RequestCommand(). ( NX_REQUEST_LAUNCHER_SHOW ret : %d )\n", iRet );
	}
}

static void cbStatusBack( void *pObj )
{
	MainWindow *pW = (MainWindow *)pObj;
	pW->SaveInfo();
	pW->StopVideo();
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

	//if(type == video)-->add
	if( !strcmp("video", cFileType ))
	{
		(((MainWindow*)pObj)->GetFileList())->AddItem( QString::fromUtf8(cFilePath) );
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_pNxPlayer(NULL)
	, m_bSubThreadFlag(false)
	, m_iScrWidth(DSP_FULL_WIDTH)
	, m_iScrHeight(DSP_FULL_HEIGHT)
	, m_iVolValue(30)
	, m_iDuration(0)
	, m_bIsInitialized(false)
	, m_pStatusBar(NULL)
	, m_bSeekReady(false)
	, m_bVoumeCtrlReady(false)
	, m_bButtonHide(false)
	, ui(new Ui::MainWindow)
	, m_iCurFileListIdx (0)
	, m_bTurnOffFlag(false)
	, m_bStopRenderingFlag(false)
	, m_bFocusTransientLossFlag(false)
	, m_bTryFlag(false)
	, m_pPlayListWindow(NULL)
	, m_bWillShow(false)
	, m_bRequestFocus(false)
{
	m_pNxPlayer = new CNX_MoviePlayer();
	UpdateFileList();

	m_pIConfig = GetConfigHandle();
	installEventFilter( this );
	NX_PacpClientStart(this);
	//	Connect Solt Functions
	connect(&mediaStateCb, SIGNAL(mediaStatusChanged(int)), SLOT(statusChanged(int)));
	pMainWindow = this;

	m_pTimer = new QTimer();

	//Update position timer
	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(DoPositionUpdate()));
	//Update Subtitle
	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(subTitleDisplayUpdate()));

	setAttribute(Qt::WA_AcceptTouchEvents, true);

	m_pTimer->start(300);

	//UI Setting
	ui->setupUi(this);

	m_pStatusBar = new CNX_StatusBar( this );
	m_pStatusBar->move( 0, 0 );
	m_pStatusBar->resize( this->size().width(), this->size().height() * 1 / 10 );
	m_pStatusBar->RegOnClickedHome( cbStatusHome );
	m_pStatusBar->RegOnClickedBack( cbStatusBack );
	m_pStatusBar->SetTitleName( "Nexell Video Player" );
	// Example for volume control & bluetooth control on the status bar.
	{
		char* pBuf = NULL;
		if(0 > m_pIConfig->Open("/nexell/daudio/NxVideoPlayer/config.xml"))
		{
			NXLOGW("xml open err\n");
		}else
		{
			//load audio volume
			if(0 > m_pIConfig->Read("volume",&pBuf))
			{
				NXLOGW("xml read pos err\n");
			}else
				m_iVolValue = atoi(pBuf);

			m_pIConfig->Close();
		}
	}
	m_pStatusBar->SetVolume(m_iVolValue);

	ui->glVideoFrame->setMainWindow(this);
	ui->volumelabel->setStyleSheet("QLabel { color : white; }");
	ui->durationlabel->setStyleSheet("QLabel { color : white; }");
	ui->appNameLabel->setStyleSheet("QLabel { color : white; }");
	ui->subTitleLabel->setStyleSheet("QLabel { color : white; }");
	ui->subTitleLabel2->setStyleSheet("QLabel { color : white; }");

	//Init Volume
	ui->volumeProgressBar->setValue(m_iVolValue);
	ui->volumelabel->setText(QString("%1").arg(m_iVolValue));
}

MainWindow::~MainWindow()
{
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
		NXLOGW("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
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

	//audio volume
	{
		char pVolume[sizeof(int)] = {};
		sprintf(pVolume, "%d", m_iVolValue);
		if(0 > m_pIConfig->Write("volume", pVolume))
		{
			NXLOGE("xml write audio volume err\n");
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
void MainWindow::StorageRemoved()
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

	if(NULL != m_pPlayListWindow)
	{
		m_listMutex.Lock();
		m_pPlayListWindow->setList(&m_FileList);
		m_listMutex.Unlock();
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
		if(m_bWillShow)
		{
			m_bWillShow = false;

			PlaySeek();
		}
		if(GetVideoMuteStatus())
		{
			NXLOGV("===========VideoMute stop==========\n");
			VideoMuteStop();
		}
	}

	if(event->type() == QEvent::WindowDeactivate)
	{
		if( (!GetVideoMuteStatus()) && (NULL == m_pPlayListWindow) )
		{
			NXLOGV("===========VideoMute Start==========\n");
			VideoMuteStart();
		}
		if( m_bRequestFocus )
		{
			NXLOGV(">>>>> Video Player Close()!!!!!\n");
			this->close();
		}
	}

	return QMainWindow::eventFilter(watched, event);
}


void MainWindow::showEvent(QShowEvent* event)
{
	Q_UNUSED( event );
	//Initialization of mediaPlayer must be done after QMainWindow::show() is done
	m_bWillShow = true;

	if(!isHidden())
	{
		return;
	}

	int32_t iRet = NX_RequestCommand( NX_REQUEST_PROCESS_SHOW );
	if( NX_REPLY_DONE != iRet )
	{
		NXLOGW( "Fail, NX_RequestCommand(). ( NX_REQUEST_PROCESS_SHOW iRet: %d )\n", iRet );
	}

	QMainWindow::show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	Q_UNUSED( event );

	int32_t iRet = NX_RequestCommand( NX_REQUEST_FOCUS_AUDIO_LOSS );
	if( NX_REPLY_DONE != iRet )
	{
		NXLOGW( "Fail, NX_RequestCommand(). ( NX_REQUEST_FOCUS_AUDIO_LOSS iRet: %d )\n", iRet );
	}

	if(isActiveWindow())
	{
		iRet = NX_RequestCommand( NX_REQUEST_FOCUS_VIDEO_LOSS );
		if( NX_REPLY_DONE != iRet )
		{
			NXLOGW( "Fail, NX_RequestCommand(). ( NX_REQUEST_FOCUS_VIDEO_LOSS iRet: %d )\n", iRet );
		}
	}

	iRet = NX_RequestCommand(NX_REQUEST_PROCESS_REMOVE);
	if( NX_REPLY_DONE != iRet )
	{
		NXLOGW( "Fail, NX_RequestCommand(). ( NX_REQUEST_PROCESS_REMOVE iRet: %d )\n", iRet );
	}

	QMainWindow::close();
}

////////////////////////////////////////////////////////////////////
//
//      Update Player Progress Bar & Volume Progress Bar
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
				if( StoppedState !=m_pNxPlayer->GetState() )
				{
					double ratio = (double)(x-minX)/(double)width;
					qint64 position = ratio * m_iDuration;
					ui->glVideoFrame->setSeekStatus(true);
					SeekVideo( position );

					//seek subtitle
					ui->subTitleLabel->setText("");
					ui->subTitleLabel2->setText("");
					m_pNxPlayer->SeekSubtitle(position);

					ui->glVideoFrame->setSeekStatus(false);
					NXLOGD("Position = %lld", position);
				}
				NXLOGD("Do Seek !!!");
				m_pTimer->start(300);
			}
		}
		m_bSeekReady = false;
	}
	else
	{
		if( (minX<=x && x<=maxX) && (minY<=y && y<=maxY) )
		{
			m_bSeekReady = true;
			NXLOGD("Ready to Seek\n");
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
			if( m_bVoumeCtrlReady )
			{
				double ratio = (double)(maxY-y)/(double)height;
				qint64 position = ratio * VOLUME_MAX;
				m_iVolValue = position;
				ui->volumeProgressBar->setValue(m_iVolValue);
				ui->volumelabel->setText(QString("%1").arg(m_iVolValue));
				if( m_pNxPlayer && StoppedState !=m_pNxPlayer->GetState() )
					SetVideoVolume(m_iVolValue);

				NXLOGI("Change Volume (%lld) !!!", position);
			}
		}
		m_bVoumeCtrlReady = false;
	}
	else
	{
		if( (minX<=x && x<=maxX) && (minY<=y && y<=maxY) )
		{
			m_bVoumeCtrlReady = true;
			NXLOGI("Ready to Volume Control\n");
		}
		else
		{
			m_bVoumeCtrlReady = false;
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

void MainWindow::on_prevButton_released()
{
	PlayPreviousVideo();
}

void MainWindow::on_playButton_released()
{
	PlayVideo();
}

void MainWindow::on_pauseButton_released()
{
	PauseVideo();
}

void MainWindow::on_nextButton_released()
{
	PlayNextVideo();
}

void MainWindow::on_stopButton_released()
{
	StopVideo();
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
	//qDebug(">> %s() Out %s", __func__, tStr.toStdString().c_str());
}

void MainWindow::statusChanged(int eventType)
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
			PlayNextVideo();
			break;
		}
		case MP_MSG_ERR_OPEN_AUDIO_DEVICE:
		{
			NXLOGE("******** MP_MSG_ERR_OPEN_AUDIO_DEVICE\n");
			PlayNextVideo();
			break;
		}
	}
}

bool MainWindow::StopVideo()
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
	ui->glVideoFrame->deInit();
	return true;
}

bool MainWindow::CloseVideo()
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

bool MainWindow::SetVideoVolume( int32_t volume )
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

bool MainWindow::PlayNextVideo()
{
	StopVideo();

	//	find next index
	if(0 != m_FileList.GetSize())
	{
		m_iCurFileListIdx = (m_iCurFileListIdx+1) % m_FileList.GetSize();
	}
	return PlayVideo();
}

bool MainWindow::PlayPreviousVideo()
{
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

void MainWindow::PlaySeek()
{
	bool seekflag = false;
	int iSavedPosition = 0;
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

bool MainWindow::PlayVideo()
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

	if(PausedState == m_pNxPlayer->GetState())
	{
		m_pNxPlayer->Play();
		return true;
	}

	if(StoppedState == m_pNxPlayer->GetState())
	{
		m_listMutex.Lock();
		if( 0 < m_FileList.GetSize() )
		{
			int iResult = -1;
			int iTryCount = 0;
			int iMaxCount = m_FileList.GetSize();
			while(0 > iResult)
			{
				if(isActiveWindow())
				{
					m_statusMutex.Lock();
					m_bStopRenderingFlag = false;
					m_statusMutex.Unlock();
				}
				iResult = m_pNxPlayer->InitMediaPlayer( cbEventCallback,
														NULL,
														m_FileList.GetList(m_iCurFileListIdx).toStdString().c_str(),
														MP_TRACK_VIDEO,
														//&cbUpdateRender
														NULL
														);
				iTryCount++;
				if(0 == iResult)
				{
					NXLOGI("\n\n********************media init done!\n");
					m_bIsInitialized = true;

					int dspWidth = 0;
					int dspHeight = 0;
					getAspectRatio(m_pNxPlayer->GetVideoWidth(0), m_pNxPlayer->GetVideoHeight(0),m_iScrWidth, m_iScrHeight, &dspWidth, &dspHeight);
					ui->glVideoFrame->init(m_pNxPlayer->GetVideoWidth(0), m_pNxPlayer->GetVideoHeight(0),dspWidth, dspHeight);

					if( 0 == OpenSubTitle() )
						PlaySubTitle();

					if( 0 > m_pNxPlayer->Play() )
					{
						NXLOGE("NX_MPPlay() failed !!!");
						iResult = -1; //retry with next file..
					}else
					{
						//play succeed
						//Update Applications
						SetVideoVolume(m_iVolValue);

						m_iDuration = m_pNxPlayer->GetMediaDuration();
						ui->progressBar->setMaximum( m_iDuration/1000 );	//	Max Seconds
						ui->appNameLabel->setText(m_FileList.GetList(m_iCurFileListIdx));

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

bool MainWindow::PauseVideo()
{
	if(NULL == m_pNxPlayer)
	{
		NXLOGE("%s(), line: %d, m_pNxPlayer is NULL \n", __FUNCTION__, __LINE__);
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

bool MainWindow::SeekVideo( int32_t mSec )
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

bool MainWindow::SetVideoMute(bool iStopRendering)
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

bool MainWindow::VideoMuteStart()
{
	SetVideoMute(true);

	if(m_pNxPlayer->IsCbQtUpdateImg())
	{
		ui->glVideoFrame->deInit();
	}
	else
	{
		NXLOGI("==============drm dsp deInit++++++++++\n");
		m_pNxPlayer->DrmVideoMute(true);
		NXLOGI("==============drm dsp deInit----------\n");
	}

	return true;
}

bool MainWindow::VideoMuteStop()
{
	int dspWidth = 0;
	int dspHeight = 0;

	if(m_pNxPlayer->IsCbQtUpdateImg())
	{
		getAspectRatio(m_pNxPlayer->GetVideoWidth(0), m_pNxPlayer->GetVideoHeight(0),m_iScrWidth, m_iScrHeight, &dspWidth, &dspHeight);
		ui->glVideoFrame->init(m_pNxPlayer->GetVideoWidth(0), m_pNxPlayer->GetVideoHeight(0),dspWidth, dspHeight);
	}
	else
	{
		NXLOGI("==============drm dsp Init++++++++++\n");
		m_pNxPlayer->DrmVideoMute(false);
		NXLOGI("==============drm dsp Init----------\n");
	}

	SetVideoMute(false);

	return true;
}

bool MainWindow::VideoMute()
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

bool MainWindow::GetVideoMuteStatus()
{
	return m_bStopRenderingFlag;
}

void MainWindow::ImageUpdate(void *pImg)
{
	m_statusMutex.Lock();
	if( m_bStopRenderingFlag )
	{
		m_statusMutex.Unlock();
		//printf("%s, ----inputMapping skipped!\n",__FUNCTION__);
		return;
	}
	m_statusMutex.Unlock();
	if (pImg)
	{
		ui->glVideoFrame->inputMapping(pImg);
	}
}

void MainWindow::displayTouchEvent()
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
		ui->volumelabel->hide();
		ui->volumeProgressBar->hide();
		ui->durationlabel->hide();
		ui->appNameLabel->hide();
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
		ui->volumelabel->show();
		ui->volumeProgressBar->show();
		ui->durationlabel->show();
		ui->appNameLabel->show();
		m_pStatusBar->show();
		m_bButtonHide = false;
	}
}

//
//		Play Util
//
void MainWindow::getAspectRatio(int srcWidth, int srcHeight,
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
void MainWindow::on_playListButton_released()
{
	if(NULL == m_pPlayListWindow)
	{
		m_pPlayListWindow = new PlayListWindow(this);
		connect(m_pPlayListWindow, SIGNAL(accepted()), this, SLOT(slotPlayListWindowAccepted()));
		connect(m_pPlayListWindow, SIGNAL(rejected()), this, SLOT(slotPlayListWindowRejected()));
	}
	m_pPlayListWindow->show();

	m_listMutex.Lock();
	m_pPlayListWindow->setList(&m_FileList);
	m_listMutex.Unlock();

	m_pPlayListWindow->setCurrentIndex(m_iCurFileListIdx);
}

void MainWindow::slotPlayListWindowAccepted()
{
	m_iCurFileListIdx = m_pPlayListWindow->getCurrentIndex();
	StopVideo();
	PlayVideo();
	m_pPlayListWindow = NULL;
}

void MainWindow::slotPlayListWindowRejected()
{
	m_pPlayListWindow = NULL;
}

//
// Subtitle Display Routine
//

void MainWindow::subTitleDisplayUpdate()
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


int MainWindow::OpenSubTitle()
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

void MainWindow::PlaySubTitle()
{
	m_bSubThreadFlag = true;
}

void MainWindow::StopSubTitle()
{
	if(m_bSubThreadFlag)
	{
		m_bSubThreadFlag = false;
	}

	m_pNxPlayer->CloseSubtitle();

	ui->subTitleLabel->setText("");
	ui->subTitleLabel2->setText("");
}

