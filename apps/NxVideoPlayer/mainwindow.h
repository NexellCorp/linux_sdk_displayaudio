#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QTouchEvent>
#include <QQuickWidget>

// status bar
#include <CNX_StatusBar.h>

#include "CNX_FileList.h"
#include "CNX_MoviePlayer.h"

#include "CNX_Util.h"
#include "playlistwindow.h"

#include <QEvent>
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
#define VOLUME_MIN  0
#define VOLUME_MAX  100

#include <QObject>
class CallBackSignal : public QObject
{
	Q_OBJECT

public:
	CallBackSignal() {}

public slots:
	void statusChanged(int eventType)
	{
		emit mediaStatusChanged(eventType);
	}

signals:
	void mediaStatusChanged(int newValue);
};

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
	//	Mouse Event
	//
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	void getAspectRatio(int srcWidth, int srcHeight,
						int scrWidth, int scrHeight,
						int *pWidth, int *pHeight);
	void ImageUpdate(void *pImg);
	void displayTouchEvent();

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

	//
	//	MediaPlayer
	bool PauseVideo();
	bool StopVideo();
	bool PlayVideo();
	bool CloseVideo();
	bool SeekVideo( int32_t iSec );
	bool SetVideoVolume( int32_t iVolume);
	bool PlayNextVideo();
	bool PlayPreviousVideo();
	bool SetVideoMute(bool iVideoMute);
	bool GetVideoMuteStatus();
	bool VideoMute();
	bool VideoMuteStart();
	bool VideoMuteStop();
	void PlaySeek();

	//
	//	SubTitle
	int	OpenSubTitle();
	void PlaySubTitle();
	void StopSubTitle();

private:
	CNX_MoviePlayer	*m_pNxPlayer;
	QTextCodec*		m_pCodec;
	bool			m_bSubThreadFlag;
	int				m_iScrWidth;
	int				m_iScrHeight;
	int				m_iVolValue;
	qint64			m_iDuration;
	QTimer			*m_pTimer;
	bool			m_bIsInitialized;
	NX_CMutex		m_statusMutex;
	//	UI Status Bar
	CNX_StatusBar* m_pStatusBar;

	//	Progress Bar
	bool	m_bSeekReady;
	bool	m_bVoumeCtrlReady;
	bool	m_bButtonHide;

	//	Override show/hide event handler
protected:
	bool eventFilter(QObject *watched, QEvent *event);

	void showEvent(QShowEvent* event);
	void closeEvent(QCloseEvent* event);

private:
	void UpdateDurationInfo(int64_t position, int64_t duration);

	//  Update Progress Bar
private:
	void updateProgressBar(QMouseEvent *event, bool bReleased);
	void updateVolumeBar(QMouseEvent *event, bool bReleased);

private slots:
	void subTitleDisplayUpdate();
	void statusChanged(int eventType);
	void DoPositionUpdate();

	void on_prevButton_released();
	void on_playButton_released();
	void on_pauseButton_released();
	void on_nextButton_released();
	void on_stopButton_released();

	//	Playlist Button
	void on_playListButton_released();
	void slotPlayListWindowAccepted();
	void slotPlayListWindowRejected();

public:
	//	for button control
	QTime m_Time;
	int32_t m_iPrvTime;
	int32_t m_iBtnPrvTime;

private:
	Ui::MainWindow *ui;

	int		m_iCurFileListIdx;	//index of media list
	CNX_FileList	m_FileList;	//media list
	NX_IConfig*		m_pIConfig;	//xml
	bool	m_bTurnOffFlag;		//for close control
	bool	m_bStopRenderingFlag;//flag for stopping rendering callback
	bool	m_bFocusTransientLossFlag;	//for FocusTransientLoss
	int		m_iPrevState;		//for FocusTransientLoss
	bool	m_bTryFlag;			//try playing back last status
	NX_CMutex	m_listMutex;	//for storage event
	PlayListWindow* m_pPlayListWindow;
	bool	m_bWillShow;
	bool 	m_bRequestFocus;
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
