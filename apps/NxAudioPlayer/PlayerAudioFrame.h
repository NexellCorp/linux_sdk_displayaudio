#ifndef PLAYERAUDIOFRAME_H
#define PLAYERAUDIOFRAME_H

#include <QFrame>

#include <QMessageBox>
#include <QMediaMetaData>
#include <QTime>
#include <QTimer>
#include <QMouseEvent>
#include <QImage>
#include <QImageReader>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

// status bar
#include <CNX_StatusBar.h>

//	Player signal handler
#include "CAudioPlayerSignals.h"
#include "CNX_FileList.h"
#include "PlayListAudioFrame.h"
//	Media Player
#include "CNX_AudioPlayer.h"

#include <NX_Type.h>
//	xml config
#include "NX_IConfig.h"
//	scanner
#include <NX_DAudioUtils.h>
#define USE_SW_VOLUME	1

#define VOLUME_MIN  0
#define VOLUME_MAX  100

namespace Ui {
class PlayerAudioFrame;
}

class PlayerAudioFrame : public QFrame
{
    Q_OBJECT

public:
    explicit PlayerAudioFrame(QWidget *parent = 0);
    ~PlayerAudioFrame();

protected:
    bool event(QEvent *e);

private:
    void TerminateEvent(NxTerminateEvent *e);
    void StatusHomeEvent(NxStatusHomeEvent *e);
    void StatusBackEvent(NxStatusBackEvent *e);
    void StatusVolumeEvent(NxStatusVolumeEvent *e);


public:
    void RegisterRequestTerminate(void (*cbFunc)(void));
    void RegisterRequestVolume(void (*cbFunc)(void));
    void RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk));

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
    CNX_AudioPlayer	*m_pNxPlayer;		//	Media Player
    QTimer			m_PosUpdateTimer;
    int64_t			m_iDuration;
    int32_t			m_iVolumeValue;		//	Player S/W Volume
    bool			m_bIsInitialized;

    static void PlayerCallbackStub( void *privateDesc, unsigned int eventType, unsigned int eventData, unsigned int dataSize ){
        ((PlayerAudioFrame *)privateDesc)->PlayerCallback(eventType, eventData, dataSize);
    }
    void PlayerCallback( unsigned int eventType, unsigned int eventData, unsigned int dataSize );

    CAudioPlayerSignals m_PlayerSignals;

public:
    bool			PauseAudio();
    bool			StopAudio();
    bool			PlayAudio();
    bool			CloseAudio();
    bool			SeekAudio( int32_t mSec );
    bool			SetAudioVolume( int32_t volume );
    bool			PlayNextAudio();
    bool			PlayPreviousAudio();
    void			PlaySeek();
    int 			GetFileListSize();

    //	Override show/hide Event
protected:
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void MediaPlayerCallback( int eventType );

private:
    //	UI Status Bar
    CNX_StatusBar* m_pStatusBar;

    QGraphicsScene m_Scene;
    QGraphicsPixmapItem m_AlbumArt;

    //	Progress Bar
    bool	m_bSeekReady;

private:
    void UpdateDurationInfo(int64_t position, int64_t duration);
    //	Update Progress Bar
    void updateProgressBar(QMouseEvent *event, bool bReleased);

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
    void slotPlayListFrameAccept();
    void slotPlayListFrameReject();


private:
    int32_t	m_iCurFileListIdx;		//index of media list
    CNX_FileList	m_FileList; 	//media list
    NX_IConfig*	m_pIConfig;         //xml
    bool	m_bTurnOffFlag;         //for close control
    bool	m_bTryFlag;             //try playing back last status
    pthread_mutex_t	m_listMutex;    //for storage event
    PlayListAudioFrame* m_pPlayListFrame;

    // Terminate
    void (*m_pRequestTerminate)(void);
    void (*m_pRequestLauncherShow)(bool *bOk);
    void (*m_pRequestVolume)(void);

private:
    Ui::PlayerAudioFrame *ui;
};

#endif // PLAYERAUDIOFRAME_H
