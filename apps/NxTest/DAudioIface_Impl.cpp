#include <DAudioIface.h>
#include "Form.h"

#ifdef CONFIG_TEST2
#	define LOG_TAG "[NxTest2]"
#else
#	define LOG_TAG "[NxTest1]"
#endif
#include <NX_Log.h>

void Init(void *pObj, const char *pArgs)
{
	(void)pArgs;
	Form *p = Form::GetInstance(pObj);
	if (!p)
	{
		NXLOGE("[%s] Instance does not exists.", __FUNCTION__);
		return;
	}

	if (!p->Initialize())
	{
		NXLOGE("[%s] Initialize().", __FUNCTION__);
		return;
	}
}

void IsInit(bool *bOk)
{
	*bOk = Form::GetInstance() ? true : false;
}

void deInit()
{
	Form::DestroyInstance();
}

void Show()
{
	Form *p = Form::GetInstance();
	if (p)
		p->show();
}

void Hide()
{
	Form *p = Form::GetInstance();
	if (p)
		p->hide();
}

void Raise()
{
	Form *p = Form::GetInstance();
	if (p)
	{
		if (p->isHidden())
		{
			NXLOGI("[%s] <HIDDEN>", __FUNCTION__);
			p->show();
		}

		p->raise();
	}
}

void Lower()
{
	NXLOGI("[%s]", __FUNCTION__);
	Form *p = Form::GetInstance();
	if (p)
		p->lower();
}

// Send Message
void SendMessage(const char *pMsg, int32_t iMsgSize)
{
	Form *p = Form::GetInstance();
	if (p)
		p->SendMessage(pMsg);

	(void)iMsgSize;
}

void RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize))
{
	Form::RegisterRequestSendMessage(cbFunc);
}

// Popup Message
void RegisterRequestPopupMessage(void (*cbFunc)(PopupMessage *, bool *))
{
	Form::RegisterRequestPopupMessage(cbFunc);
}

void RegisterRequestExpirePopupMessage(void (*cbFunc)())
{
	Form::RegisterRequestExpirePopupMessage(cbFunc);
}

void PopupMessageResponse(bool bOk)
{
	Form *p = Form::GetInstance();
	if (p)
		p->PopupMessageResponse(bOk);
}

void RegisterRequestNotification(void (*cbFunc)(PopupMessage *))
{
	Form::RegisterRequestNotification(cbFunc);
}

void RegisterRequestExpireNotification(void (*cbFunc)())
{
	Form::RegisterRequestExpireNotification(cbFunc);
}

void NotificationResponse(bool bOk)
{
	Form *p = Form::GetInstance();
	if (p)
		p->NotificationResponse(bOk);
}

// Audio Focus
void RequestAudioFocus(FocusType eType, FocusPriority ePriority, bool *bOk)
{
	Form *p = Form::GetInstance();
	if (p)
		p->RequestAudioFocus(eType, ePriority, bOk);
}

void RegisterRequestAudioFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	Form::RegisterRequestAudioFocus(cbFunc);
}

void RequestAudioFocusTransient(FocusPriority ePriority, bool *bOk)
{
	Form *p = Form::GetInstance();
	if (p)
		p->RequestAudioFocusTransient(ePriority, bOk);
}

void RegisterRequestAudioFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	Form::RegisterRequestAudioFocusTransient(cbFunc);
}

void RegisterRequestAudioFocusLoss(void (*cbFunc)(void))
{
	Form::RegisterRequestAudioFocusLoss(cbFunc);
}

// Video Focus
void RequestVideoFocus(FocusType eType, FocusPriority ePriority, bool *bOk)
{
	Form *p = Form::GetInstance();
	if (p)
		p->RequestVideoFocus(eType, ePriority,bOk);
}

void RegisterRequestVideoFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	Form::RegisterRequestVideoFocus(cbFunc);
}

void RequestVideoFocusTransient(FocusPriority ePriority, bool *bOk)
{
	Form *p = Form::GetInstance();
	if (p)
		p->RequestVideoFocusTransient(ePriority,bOk);
}

void RegisterRequestVideoFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	Form::RegisterRequestVideoFocusTransient(cbFunc);
}

void RegisterRequestVideoFocusLoss(void (*cbFunc)(void))
{
	Form::RegisterRequestVideoFocusLoss(cbFunc);
}

void RegisterRequestTerminate(void (*cbFunc)())
{
	Form::RegisterRequestTerminate(cbFunc);
}

void MediaEventChanged(NxMediaEvent eEvent)
{
	Form *p = Form::GetInstance();
	if (p)
		p->MediaEventChanged(eEvent);
}

void BackButtonClicked()
{
	Form *p = Form::GetInstance();
	if (p)
	{
		p->BackButtonClicked();
	}
}
