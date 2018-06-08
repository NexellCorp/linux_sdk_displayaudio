#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QMessageBox>
#include <QMediaMetaData>
#include <QTime>
#include <QTimer>
#include <QMouseEvent>
#include <QImage>
#include <QImageReader>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QQuickWidget>

// status bar
#include <CNX_StatusBar.h>

//	Player signal handler
#include "CAudioPlayerSignals.h"
#include "CNX_FileList.h"
#include "playlistwindow.h"
//	Media Player
#include "CNX_MoviePlayer.h"

//	IPC
#include <NX_PacpClient.h>
#include <NX_Type.h>
#include <INX_IpcManager.h>
#include <NX_IpcPacket.h>
#include <NX_IpcUtils.h>
//	xml config
#include "NX_IConfig.h"
//	scanner
#include <NX_DAudioUtils.h>
#define USE_SW_VOLUME	1

#define VOLUME_MIN  0
#define VOLUME_MAX  100

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();


	//
	//	Mouse Event for Progress Bar
	//
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	//	for focus, close control)
	void SetTurnOffFlag(bool bFlag);
	void SetFocusTransientLossFlag(bool bFlag);
	bool GetFocusTransientLossFlag();
	void RestoreState();

	//	xml (save previous state)
	int SaveInfo();
	bool SeekToPrev(int* iSavedPosition, int* iFileIdx);

	//	Storage Event
	void StorageRemoved();
	void StorageScanDone();
	void UpdateFileList();
	CNX_FileList* GetFileList();

	//	thumbnail
	void UpdateAlbumInfo();

	//////////////////////////////////////////////////////////////////////////
	//
	//	Media Player Control
	//
	//////////////////////////////////////////////////////////////////////////
private:
	CNX_MoviePlayer	*m_pNxPlayer;		//	Media Player
	QTimer			m_PosUpdateTimer;
	int64_t			m_iDuration;
	int32_t			m_iVolumeValue;		//	Player S/W Volume
	bool			m_bIsInitialized;

	static void PlayerCallbackStub( void *privateDesc, unsigned int eventType, unsigned int eventData, unsigned int dataSize ){
		((MainWindow *)privateDesc)->PlayerCallback(eventType, eventData, dataSize);
	}
	void PlayerCallback( unsigned int eventType, unsigned int eventData, unsigned int dataSize );

	CAudioPlayerSignals m_PlayerSignals;

public:
	bool			Play();
	bool			Stop();
	bool			Pause();
	bool			Seek( int32_t mSec );
	bool			CloseAudio();
	bool			SetVolume( int32_t volume );

	bool			PlayNextMusic();
	bool			PlayPreviousMusic();

	void			PlaySeek();
	//	Override show/hide Event
protected:
	bool eventFilter(QObject *watched, QEvent *event);

	void showEvent(QShowEvent* event);
	void closeEvent(QCloseEvent* event);

private slots:
	void MediaPlayerCallback( int eventType );

private:
	//	UI Status Bar
	CNX_StatusBar* m_pStatusBar;

	QGraphicsScene m_Scene;
	QGraphicsPixmapItem m_AlbumArt;

	//	Progress Bar
	bool	m_bSeekReady;
	bool	m_bVolumeCtrlReady;

private:
	void UpdateDurationInfo(int64_t position, int64_t duration);
	//	Update Progress Bar
	void updateProgressBar(QMouseEvent *event, bool bReleased);
	void updateVolumeBar(QMouseEvent *event, bool bReleased);

private slots:
	//
	//	QMediaPlayer's Slots
	//
	void DoPositionUpdate();

	//
	//	Button Events
	//
	//	player control
	void on_prevButton_released();
	void on_playButton_released();
	void on_pauseButton_released();
	void on_nextButton_released();
	void on_stopButton_released();

	//	Playlist Button
	void on_playListButton_released();
	void slotPlayListWindowAccepted();
	void slotPlayListWindowRejected();

private:
	Ui::MainWindow *ui;

	int	m_iCurFileListIdx;		//index of media list
	CNX_FileList	m_FileList;	//media list
	NX_IConfig*	m_pIConfig;		//xml
	bool	m_bTurnOffFlag;		//for close controll
	bool	m_bFocusTransientLossFlag;	//for FocusTransientLoss
	int		m_iPrevState;		//for FocusTransientLoss
	bool	m_bTryFlag;			//try playing back last status
	pthread_mutex_t	m_listMutex;//for storage event
	PlayListWindow* m_pPlayListWindow;

	bool	m_bRequestFocus;
};

class NxEvent : QEvent
{
public:
	int32_t m_iEventType;

	NxEvent(QEvent::Type type) : QEvent(type)
	{
	}
};

#endif // MAINWINDOW_H
