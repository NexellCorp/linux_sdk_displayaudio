#ifndef FRAME_H
#define FRAME_H

#include <QFrame>
#include <DAudioIface.h>

#include "BTCommandProcessor.h"
#include "NxEvent.h"

namespace Ui {
class Frame;
}

class Frame : public QFrame
{
	Q_OBJECT

signals:
	void signalCommandToServer(QString command);

private slots:
	void slotCommandFromServer(QString command);

public:
	enum UIState {
		UIState_Playing,
		UIState_Paused,
		UIState_Stopped
	};

	static Frame* GetInstance(void *pObj);

	static Frame* GetInstance();

	static void DestroyInstance();

	bool Initialize();

	void BackButtonClicked();

private slots:
	void on_BUTTON_PLAY_START_clicked();

	void on_BUTTON_PLAY_PAUSE_clicked();

	void on_BUTTON_PLAY_PREV_clicked();

	void on_BUTTON_PLAY_NEXT_clicked();

private:
	explicit Frame(QWidget *parent = 0);
	~Frame();

protected:
	bool event(QEvent *e);

	void StatusHomeEvent(NxStatusHomeEvent *e);

	void StatusBackEvent(NxStatusBackEvent *e);

	void StatusVolumeEvent(NxStatusVolumeEvent *e);

public:
	void Init(void *pObj);

	void IsInit(bool *bOk);

	void deInit();

	void Show();

	void Hide();

	void Raise();

	void Lower();

	static void RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk));

	static void RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize));

	void SendMessage(QString msg);

	void RequestVideoFocus(FocusType eType, FocusPriority ePriority, bool *bOk);

	static void RegisterRequestVideoFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk));

	void RequestVideoFocusTransient(FocusPriority ePriority, bool *bOk);

	static void RegisterRequestVideoFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk));

	static void RegisterRequestVideoFocusLoss(void (*cbFunc)(void));

	static void RegisterRequestTerminate(void (*cbFunc)(void));

private:
	void setUIState(UIState state);

	void updateToUIForMediaElements(QStringList& tokens);

	void updateToUIForPlayPosition(QStringList& tokens);

	void updateToUIForPlayStatus(QStringList& tokens);

	void updateToUIForPlayInformation(QStringList& tokens);

private:
	static Frame *m_spInstance;

	UIState m_UIState;

	BTCommandProcessor *m_pCommandProcessor;

	// Send Message
	static void (*m_pRequestSendMessage)(const char *pDst, const char *pMsg, int32_t iMsgSize);

	// Video
	static void (*m_pRequestVideoFocus)(FocusPriority ePriority, bool *bOk);
	static void (*m_pRequestVideoFocusTransient)(FocusPriority ePriority, bool *bOk);
	static void (*m_pRequestVideoFocusLoss)(void);

	// Terminate
	static void (*m_pRequestTerminate)(void);

	// Focus
	bool m_bHasVideoFocus;

	bool m_bInitialized;

private:
	Ui::Frame *ui;
};

#endif // FRAME_H
