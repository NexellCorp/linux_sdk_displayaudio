#ifndef FORM_H
#define FORM_H

#include <QFrame>
#include <NX_Type.h>
#include "NxEvent.h"

namespace Ui {
class Form;
}

class Form : public QFrame
{
	Q_OBJECT

public:
	static Form *GetInstance(void *pObj);

	static Form *GetInstance();

	static void DestroyInstance();

	bool Initialize();

	// Launcher Show
	static void RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk));

	// Message
	void SendMessage(QString msg);

	static void RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize));

	// Popup Message
	static void RegisterRequestPopupMessage(void (*cbFunc)(PopupMessage *, bool *));

	static void RegisterRequestExpirePopupMessage(void (*cbFunc)());

	void PopupMessageResponse(bool bOk);

	// Audio Focus
	void RequestAudioFocus(FocusType eType, FocusPriority ePriority, bool *bOk);

	static void RegisterRequestAudioFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk));

	void RequestAudioFocusTransient(FocusPriority ePriority, bool *bOk);

	static void RegisterRequestAudioFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk));

	static void RegisterRequestAudioFocusLoss(void (*cbFunc)(void));

	// Video Focus
	void RequestVideoFocus(FocusType eType, FocusPriority ePriority, bool *bOk);

	static void RegisterRequestVideoFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk));

	void RequestVideoFocusTransient(FocusPriority ePriority, bool *bOk);

	static void RegisterRequestVideoFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk));

	static void RegisterRequestVideoFocusLoss(void (*cbFunc)(void));

	static void RegisterRequestTerminate(void (*cbFunc)(void));

	// Media Event
	void MediaEventChanged(NxMediaEvent eEvent);

protected:
	bool event(QEvent *e);

private:
	static void cbStatusHome(void *pObj);

	static void cbStatusBack(void *pObj);

	void StatusHomeEvent(NxStatusHomeEvent *e);

	void StatusBackEvent(NxStatusBackEvent *e);

private:
	explicit Form(QWidget *parent = 0);
	~Form();

	static Form *m_spInstance;

private:
	// Launcher Show
	static void (*m_pRequestLauncherShow)(bool *bOk);

	// Message
	static void (*m_pRequestSendMessage)(const char *pDst, const char *pMsg, int32_t iMsgSize);

	// Popup Message
	static void (*m_pRequestPopupMessage)(PopupMessage *, bool *);
	static void (*m_pReqeustExpirePopupMessage)();

	// Audio
	static void (*m_pRequestAudioFocus)(FocusPriority ePriority, bool *bOk);
	static void (*m_pRequestAudioFocusTransient)(FocusPriority ePriority, bool *bOk);
	static void (*m_pRequestAudioFocusLoss)(void);

	// Video
	static void (*m_pRequestVideoFocus)(FocusPriority ePriority, bool *bOk);
	static void (*m_pRequestVideoFocusTransient)(FocusPriority ePriority, bool *bOk);
	static void (*m_pRequestVideoFocusLoss)(void);

	// Terminate
	static void (*m_pRequestTerminate)(void);

	// Focus
	bool m_bHasAudioFocus;
	bool m_bHasVideoFocus;

	bool m_bInitialized;

private slots:
	void on_BUTTON_CHECK_FOCUS_clicked();

	void on_BUTTON_AUDIOFOCUS_clicked();

	void on_BUTTON_AUDIOFOCUS_TRANSIENT_clicked();

	void on_BUTTON_AUDIOFOCUS_LOSS_clicked();

	void on_BUTTON_VIDEOFOCUS_clicked();

	void on_BUTTON_VIDEOFOCUS_TRANSIENT_clicked();

	void on_BUTTON_VIDEOFOCUS_LOSS_clicked();

	void on_BUTTON_SEND_MESSAGE_clicked();

	void on_BUTTON_POPUP_MESSAGE_clicked();

private:
	Ui::Form *ui;
};

#endif // FORM_H
