#ifndef QUICKREARCAM_H
#define QUICKREARCAM_H

#include <QFrame>
#include <QMediaPlayer>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QTouchEvent>
#include <QGraphicsScene>

#include <NX_RearCam.h>
#include <nx-v4l2.h>
#include <drm/drm_fourcc.h>
// status bar
#include <CNX_StatusBar.h>

#include "NxEvent.h"

#include "CNX_Util.h"

#include <QEvent>
#include <NX_Type.h>

//	scanner
#include <NX_DAudioUtils.h>
#define VOLUME_MIN  0
#define VOLUME_MAX  100

#include <QObject>

typedef struct NX_RGB_DRAW_INFO {
	uint32_t 	planeId;
	uint32_t 	crtcId;

	int32_t		m_iDspX;
	int32_t		m_iDspY;
	int32_t 	m_iDspWidth;
	int32_t		m_iDspHeight;

	int32_t		m_iNumBuffer;

	int32_t		uDrmFormat;

	int32_t 	drmFd;

}NX_RGB_DRAW_INFO;

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
class RearCamFrame;
}

class RearCamFrame : public QFrame
{
	Q_OBJECT

public:
	explicit RearCamFrame(QWidget *parent = 0);
	~RearCamFrame();


private:
	void TerminateEvent(NxTerminateEvent *e);

public:
	void RegisterRequestTerminate(void (*cbFunc)(void));
	void RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk));
	void RegisterRequestOpacity(void (*cbFunc)(bool));

private:
	bool m_bIsInitialized;

 public:
	bool ShowCamera();
	void HideCamera();
	bool IsShowCamera();
	void SetupUI();

protected:
	void paintEvent(QPaintEvent* event);

private:
	QGraphicsScene *scene;
	bool m_bDrawPGL;
	bool m_bShowCamera;

private:

	// Terminate
	void (*m_pRequestTerminate)(void);
	void (*m_pRequestLauncherShow)(bool *bOk);

		// Opacity
	void (*m_pRequestOpacity)(bool bOpacity);


	int SaveInfo();

public:
	NX_REARCAM_INFO vip_info;       // camera info
	DISPLAY_INFO dsp_info;          // display info for rendering camera data
	DEINTERLACE_INFO deinter_info;  // deinterlace info
	NX_RGB_DRAW_INFO pgl_dsp_info;

	int32_t pgl_enable;

private:
	Ui::RearCamFrame *ui;
};

#endif // QUICKREARCAM_H
