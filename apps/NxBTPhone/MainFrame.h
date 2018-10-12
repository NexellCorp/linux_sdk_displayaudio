#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <QFrame>
//
#include <DAudioIface.h>

#include "NxEvent.h"
#include "BTCommandProcessor.h"
#include "defines.h"

namespace Ui {
class MainFrame;
}

class MainFrame : public QFrame
{
	Q_OBJECT

signals:
	void signalCommandToServer(QString command);

private slots:
	void slotCommandFromServer(QString command);

	void slotCurrentMenuChanged(Menu eMenu);

public:
	bool Initialize(QString args);

	static MainFrame* GetInstance(void *pObj);

	static MainFrame* GetInstance();

	static void DestroyInstance();

	static void RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk));

	static void RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize));

	void SendMessage(QString msg);

	void RequestVideoFocus(FocusType eType, FocusPriority ePriority, bool *bOk);

	static void RegisterRequestVideoFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk));

	void RequestVideoFocusTransient(FocusPriority ePriority, bool *bOk);

	static void RegisterRequestVideoFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk));

	static void RegisterRequestVideoFocusLoss(void (*cbFunc)(void));

	static void RegisterRequestTerminate(void (*cbFunc)(void));

	void BackButtonClicked();

private:
	explicit MainFrame(QWidget *parent = 0);

	~MainFrame();

protected:
	bool event(QEvent *e);

	void StatusBackEvent(NxStatusBackEvent *e);

private:
	void SetCurrentMenu(Menu menu, bool update = true);

	void ProcessForCallDisconnected();

private:
	static MainFrame* m_spInstance;

	BTCommandProcessor *m_pCommandProcessor;

	Menu m_eCurrentMenu;

	// Launcher Show
	static void (*m_pRequestLauncherShow)(bool *bOk);

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

	bool m_bHasVideoFocusTransient;

	bool m_bInitialized;

	// 전화 연결이 끊어졌을 경우에 대한 판단하는 플래그
	// 해당 플래그가 켜진 상태에서 $HS#AUDIO STATUS#AUDIO CLOSED#... 명령어 수신 시, 오디오 포커스를 풀어주기 위한 목적
	bool m_bDisconnectedCall;
	bool m_bAudioClosedForHS;
	bool m_bBTConnectedForHS;

	bool m_bDisconnectedCallIsExit;

private:
	Ui::MainFrame *ui;
};

#endif // MAINFRAME_H
