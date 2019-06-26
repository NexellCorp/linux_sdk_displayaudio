#ifndef NXLAUNCHER_H
#define NXLAUNCHER_H

#include <QDialog>
#include <QWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QQueue>
#include <QTimer>
#ifndef CONFIG_NXP3220
#include <QQuickWidget>
#else
#include <page/PageStackFrame.h>
#endif

#include <media/MediaScanner.h>
#include "nxpackagemanager.h"
#include "nxappinfo.h"

#include <NX_Type.h>
#include <NxEvent.h>

namespace Ui {
class NxLauncher;
}

class NxLauncher : public QDialog
{
	Q_OBJECT

signals:
	void signalStateChanged(NxPluginInfo*);

private slots:
#ifdef CONFIG_NXP3220
	void slotResizeItemDone();
#endif

	void slotExecute(QString);

	void slotPopupMessageAccept();

	void slotPopupMessageReject();

	void slotNotificationAccept();

	void slotNotificationReject();

	void slotTimer();

	void slotPlugInUpdated(QString plugin);

	void slotSetVolume(int value);

	void slotMediaEvent(NxEventTypes eType);

	void slotStartSerivceTimer();

#ifdef CONFIG_NXP3220
	void onExecute(NxPluginInfo* pInfo);

	void onPrevPageButtonClicked();

	void onNextPageButtonClicked();
#endif

public:
	explicit NxLauncher(QWidget *parent = 0);
	~NxLauncher();

	static NxLauncher* GetInstance();

public:
	// call function from qml
	Q_INVOKABLE QVariantList getPluginInfoList();

	void LauncherShow(bool *bOk, bool bRequireRequestFocus);

protected:
	bool event(QEvent *event);

	void resizeEvent(QResizeEvent *);

private:
	// callback functions
	static void RequestLauncherShow(bool *bOk);

	static void RequestSendMessage(const char *pDst, const char *pMsg, int32_t iMsgSize);

	// Plugin Management from other plugin
	static void RequestPlugInRun(const char *pPlugin, const char *pArgs);

	static void RequestPlugInTerminate(const char *pPlugin);

	static void RequestPlugInIsRunning(const char *pPlugin, bool *bOk);

	// Audio Focus Management
	static void RequestAudioFocus(FocusPriority ePriority, bool *bOk);

	static void RequestAudioFocusTransient(FocusPriority ePriority, bool *bOk);

	static void RequestAudioFocusLoss();

	// Video Focus Management
	static void RequestVideoFocus(FocusPriority ePriority, bool *bOk);

	void VideoFocus(FocusPriority ePriority, bool *bOk);

	static void RequestVideoFocusTransient(FocusPriority ePriority, bool *bOk);

	static void RequestVideoFocusLoss();

	// Popup message
	static void RequestPopupMessage(PopupMessage *psPopup, bool *bOk);

	static void RequestExpirePopupMessage();

	void ExpirePopupMessage();

	// Notification
	static void RequestNotification(PopupMessage *psPopup);

	static void RequestExpireNotification();

	void ExpireNotification();

	//
	static void RequestTerminate();

	static void RequestVolume();

	static void RequestOpacity(bool opacity);

	void KeyEvent(NxKeyEvent* e);

	void PopupMessageEvent(NxPopupMessageEvent *e);

	void NotificationEvent(NxNotificationEvent *e);

	void VolumeControlEvent(NxVolumeControlEvent *e);

	void OpacityEvent(NxOpacityEvent *e);

	static QString FindCaller(uint32_t uiLevel);

	static void cbStatusHome(void *pObj);

	static void cbStatusBack(void *pObj);

	static void cbStatusVolume(void *pObj);

	void NextVideoFocus();

	void Terminate(QString requestor);

	void Execute(QString plugin);

	bool AddVideoFocus(QString owner);

	void RemoveVideoFocus(QString requestor);

	void UpdateTitle(QString owner);

	void BackButtonClicked();

private:
	static NxLauncher *m_spInstance;

	QMap<QString, NxPluginInfo*> m_PlugIns;

	NxPackageScanner* m_pPackageManager;

	static QQueue<QString> m_AudioFocusQueue;
	static QQueue<QString> m_VideoFocusQueue;

	QTimer m_Timer;

	MediaScanner *m_pMediaScanner;
#ifdef CONFIG_NXP3220
	PageStackFrame *m_pPageStackFrame;
	QPushButton *m_pPrevPageButton;
	QPushButton *m_pNextPageButton;
#else
	QQuickWidget *m_pLauncherWidget;
#endif

private:
	Ui::NxLauncher *ui;
};

#endif // NXLAUNCHER_H
