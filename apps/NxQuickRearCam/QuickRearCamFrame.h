#ifndef QUICKREARCAM_H
#define QUICKREARCAM_H

#include <QFrame>
#include <QMediaPlayer>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QTouchEvent>

#include <NX_RearCam.h>
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
class QuickRearCamFrame;
}

class QuickRearCamFrame : public QFrame
{
	Q_OBJECT

public:
	explicit QuickRearCamFrame(QWidget *parent = 0);
	~QuickRearCamFrame();


private:
	void TerminateEvent(NxTerminateEvent *e);

public:
	void RegisterRequestTerminate(void (*cbFunc)(void));
	void RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk));

private:
	bool			m_bIsInitialized;

 public:
	bool ShowCamera();
	void HideCamera();
	bool IsShowCamera();

private:
	bool m_bShowCamera;
	CAMERA_INFO m_CamInfo;
	DISPLAY_INFO m_DspInfo;

private:

	// Terminate
	void (*m_pRequestTerminate)(void);
	void (*m_pRequestLauncherShow)(bool *bOk);

	int SaveInfo();

private:
	Ui::QuickRearCamFrame *ui;
	NX_IConfig*		m_pIConfig;	//xml
};

#endif // QUICKREARCAM_H
