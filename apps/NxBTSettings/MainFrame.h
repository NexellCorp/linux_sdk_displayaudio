#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <QFrame>
#include <QMovie>

#include <NX_Type.h>

#include "BTCommandProcessor.h"
#include "Types.h"
#include "NxEvent.h"

namespace Ui {
class MainFrame;
}

class MainFrame : public QFrame
{
	Q_OBJECT

private slots:
	void slotCommandFromServer(QString command);

	void slotCurrentMenuChanged(Menu eMenu);

public:
	static MainFrame* GetInstance(void *pObj);

	static MainFrame* GetInstance();

	static void DestroyInstance();

	bool Initialize();

	// Message
	void SendMessage(QString msg);

	static void RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize));

	// Popup Message
	static void RegisterRequestPopupMessage(void (*cbFunc)(PopupMessage *, bool *));

	void PopupMessageResponse(bool bOk);

	// Video Focus
	void RequestVideoFocus(FocusType eType, FocusPriority ePriority, bool *bOk);

	static void RegisterRequestVideoFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk));

	void RequestVideoFocusTransient(FocusPriority ePriority, bool *bOk);

	static void RegisterRequestVideoFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk));

	static void RegisterRequestVideoFocusLoss(void (*cbFunc)(void));

	//
	static void RegisterRequestTerminate(void (*cbFunc)());

	void BackButtonClicked();

private:
	explicit MainFrame(QWidget *parent = 0);
	~MainFrame();

	void setCurrentMenu(Menu menu);

	Menu GetCurrentMenu();

	static void cbStatusHome(void *);

	static void cbStatusBack(void *);

protected:
	bool event(QEvent *e);

	void StatusHomeEvent(NxStatusHomeEvent *e);

	void StatusBackEvent(NxStatusBackEvent *e);

private:
	BTCommandProcessor *m_pCommandProcessor;

	QMovie* m_pLoadingImage;

	Menu m_Menu;

private:
	// Launcher Show
	static void (*m_pRequestLauncherShow)(bool *bOk);

	// Popup Message
	static void (*m_RequestPopupMessage)(PopupMessage *, bool *);

	// Send Message
	static void (*m_pRequestSendMessage)(const char *pDst, const char *pMsg, int32_t iMsgSize);

	// Video
	static void (*m_pRequestVideoFocus)(FocusPriority ePriority, bool *bOk);
	static void (*m_pRequestVideoFocusTransient)(FocusPriority ePriority, bool *bOk);
	static void (*m_pRequestVideoFocusLoss)(void);

	//
	static void (*m_pRequestTerminate)(void);

	// Focus
	bool m_bHasAudioFocus;
	bool m_bHasVideoFocus;

private:
	static MainFrame* m_spInstance;

	bool m_bInitialized;

private:
	Ui::MainFrame *ui;
};

#endif // MAINFRAME_H
