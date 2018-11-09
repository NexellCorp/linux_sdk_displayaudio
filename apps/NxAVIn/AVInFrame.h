#ifndef AVINFRAME_H
#define AVINFRAME_H

#include <QFrame>
#include <QMediaPlayer>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QTouchEvent>

#include <NX_AVIn.h>
#include <nx-v4l2.h>
#include <drm/drm_fourcc.h>
// status bar
#include <CNX_StatusBar.h>

#include "NxEvent.h"

#include "CNX_Util.h"

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
class AVInFrame;
}

class AVInFrame : public QFrame
{
    Q_OBJECT

// signals:
//     void signalPlayList();

public:
    explicit AVInFrame(QWidget *parent = 0);
    ~AVInFrame();

protected:
    bool event(QEvent *e);

private:
    void TerminateEvent(NxTerminateEvent *e);
    void StatusHomeEvent(NxStatusHomeEvent *e);
    void StatusBackEvent(NxStatusBackEvent *e);

public:
    void RegisterRequestTerminate(void (*cbFunc)(void));
    void RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk));
    void displayTouchEvent();


private:
    bool			m_bIsInitialized;

    CNX_StatusBar* m_pStatusBar;

    bool	m_bButtonHide;

    //	event filter
    bool eventFilter(QObject *watched, QEvent *event);

public:
    bool ShowAVIn();
    void StopAVIn();
    bool IsShowAVIn();
    void SetVideoFocus(bool m_bVideoFocusStatus);

    int32_t m_bGearStatus;
    bool m_bVideoFocus;

private:
    bool m_bShowAVIn;
    CAMERA_INFO m_CamInfo;
	DISPLAY_INFO m_DspInfo;


private:
    int		m_iCurFileListIdx;	//index of media list
    bool	m_bTurnOffFlag;		//for close control
    bool	m_bStopRenderingFlag;//flag for stopping rendering callback
    bool	m_bTryFlag;			//try playing back last status
    NX_CMutex	m_listMutex;	//for storage event
     bool 	m_bIsVideoFocus;

    // Terminate
    void (*m_pRequestTerminate)(void);
    void (*m_pRequestLauncherShow)(bool *bOk);
    void (*m_pRequestVolume)(void);

    int SaveInfo();

private:
    Ui::AVInFrame *ui;
    NX_IConfig*		m_pIConfig;	//xml
};

#endif // AVINFRAME_H
