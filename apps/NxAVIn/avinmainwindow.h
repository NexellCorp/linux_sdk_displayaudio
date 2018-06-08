#ifndef AVINMAINWINDOW_H
#define AVINMAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QTouchEvent>

//	Base UI
#include <CNX_StatusBar.h>

#include <NX_AVIn.h>
#include <QEvent>
#include <NX_Type.h>
#include <INX_IpcManager.h>
#include <NX_IpcPacket.h>
#include <NX_IpcUtils.h>

#define ENABLE_REQUEST_FOCUS_AUDIO		0

namespace Ui {
class AVInMainWindow;
}

class AVInMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit AVInMainWindow(QWidget *parent = 0);
	~AVInMainWindow();

	bool ShowAVIn();
	void StopAVIn();
	bool IsShowAVIn();
	void ToggleStatusBar();

protected:
	bool eventFilter(QObject *watched, QEvent *event);
	void showEvent( QShowEvent* event );
	void hideEvent( QHideEvent* event );
	void closeEvent( QCloseEvent* event );

private:
	Ui::AVInMainWindow *ui;

	CNX_StatusBar* m_pStatusBar;

	CAMERA_INFO m_CamInfo;
	DISPLAY_INFO m_DspInfo;

	bool m_bShowAVIn;
	bool m_bShowStatusBar;
};

class NxEvent : QEvent
{
public:
	int32_t m_iEventType;

	NxEvent(QEvent::Type type) : QEvent(type)
	{
	}
};

#endif // AVINMAINWINDOW_H
