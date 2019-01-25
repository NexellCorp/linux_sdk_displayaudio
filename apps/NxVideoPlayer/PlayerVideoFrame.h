#ifndef PLAYERVIDEOFRAME_H
#define PLAYERVIDEOFRAME_H

#include <QFrame>
#include <QMediaPlayer>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QTouchEvent>

// status bar
#include <CNX_StatusBar.h>

#include "CNX_FileList.h"
#include "CNX_MoviePlayer.h"

#include "CNX_Util.h"
#include "PlayListVideoFrame.h"

#include <QEvent>
#include <NX_Type.h>

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
class PlayerVideoFrame;
}

class PlayerVideoFrame : public QFrame
{
    Q_OBJECT

signals:
    void signalPlayList();

private slots:
    void slotPlayListFrameAccept();
    void slotPlayListFrameReject();
    void subTitleDisplayUpdate();
    void statusChanged(int eventType);
    void DoPositionUpdate();

    void on_prevButton_released();
    void on_playButton_released();
    void on_pauseButton_released();
    void on_nextButton_released();
    void on_stopButton_released();
    void on_playListButton_released();

public:
    explicit PlayerVideoFrame(QWidget *parent = 0);
    ~PlayerVideoFrame();

protected:
    bool event(QEvent *e);
    void resizeEvent(QResizeEvent *event);

private:
    void TerminateEvent(NxTerminateEvent *e);
    void StatusHomeEvent(NxStatusHomeEvent *e);
    void StatusBackEvent(NxStatusBackEvent *e);
    void StatusVolumeEvent(NxStatusVolumeEvent *e);

public:
    void RegisterRequestTerminate(void (*cbFunc)(void));
    void RegisterRequestVolume(void (*cbFunc)(void));
    void RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk));
    void getAspectRatio(int srcWidth, int srcHeight,
                        int scrWidth, int scrHeight,
                        int *pWidth, int *pHeight);
    void displayTouchEvent();

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
    int	 OpenSubTitle();
    void PlaySubTitle();
    void StopSubTitle();

private:
    CNX_MoviePlayer	*m_pNxPlayer;
    QTextCodec*		m_pCodec;
    bool			m_bSubThreadFlag;
    int				m_iVolValue;
    qint64			m_iDuration;
    QTimer			m_PosUpdateTimer;
    bool			m_bIsInitialized;
    NX_CMutex		m_statusMutex;
    //	UI Status Bar
    CNX_StatusBar* m_pStatusBar;

    //	Progress Bar
    bool	m_bSeekReady;
    bool	m_bButtonHide;

    //	event filter
    bool eventFilter(QObject *watched, QEvent *event);

private:
    void SetupUI();
    void UpdateDurationInfo(int64_t position, int64_t duration);

    //  Update Progress Bar
private:
    void updateProgressBar(QMouseEvent *event, bool bReleased);

public:
    void setVideoFocus(bool bVideoFocus);

private:
    int		m_iCurFileListIdx;	//index of media list
    CNX_FileList	m_FileList;	//media list
    NX_IConfig*		m_pIConfig;	//xml
    bool	m_bTurnOffFlag;		//for close control
    bool	m_bStopRenderingFlag;//flag for stopping rendering callback
    bool	m_bTryFlag;			//try playing back last status
    NX_CMutex	m_listMutex;	//for storage event
    PlayListVideoFrame* m_pPlayListFrame;
    bool 	m_bIsVideoFocus;

    // Terminate
    void (*m_pRequestTerminate)(void);
    void (*m_pRequestLauncherShow)(bool *bOk);
    void (*m_pRequestVolume)(void);

private:
    Ui::PlayerVideoFrame *ui;
};

#endif // PLAYERVIDEOFRAME_H
