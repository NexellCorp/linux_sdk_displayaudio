#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QEvent>
#include <unistd.h>	//	usleep
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include <NX_Type.h>
#include <INX_IpcManager.h>
#include <NX_IpcPacket.h>
#include <NX_IpcUtils.h>

#include <NX_RearCam.h>
#include <nx-v4l2.h>
#include <drm/drm_fourcc.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	bool ShowCamera();
	void HideCamera();
	bool IsShowCamera();

protected:
	//dialog event override
	bool eventFilter(QObject *watched, QEvent *event);
	void showEvent( QShowEvent* event );
	void hideEvent( QHideEvent* event );

private:
	Ui::MainWindow *ui;

	CAMERA_INFO m_CamInfo;
	DISPLAY_INFO m_DspInfo;

	bool m_bShowCamera;
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
