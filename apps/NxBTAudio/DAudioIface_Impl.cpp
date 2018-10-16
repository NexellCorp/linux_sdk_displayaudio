#include <DAudioIface.h>
#include "Frame.h"

#define LOG_TAG "[NxBTAudio]"
#include <NX_Log.h>

void Init(void *pObj, const char *pArgs)
{
	(void)pArgs;
	Frame *p = Frame::GetInstance(pObj);
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
	*bOk = Frame::GetInstance() ? true : false;
}

void deInit()
{
	Frame::DestroyInstance();
}

void Show()
{
	NXLOGI("[%s]", __FUNCTION__);
	Frame *p = Frame::GetInstance();
	if (p)
		p->show();
}

void Hide()
{
	NXLOGI("[%s]", __FUNCTION__);
	Frame *p = Frame::GetInstance();
	if (p)
		p->hide();
}

void Raise()
{
	NXLOGI("[%s]", __FUNCTION__);
	Frame *p = Frame::GetInstance();
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
	Frame *p = Frame::GetInstance();
	if (p)
		p->lower();
}

// Message
void SendMessage(const char *pSrc, const char *pMsg, int32_t iMsgSize)
{
	Frame *p = Frame::GetInstance();
	if (p)
		p->SendMessage(pMsg);
}

void RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize))
{
	Frame::RegisterRequestSendMessage(cbFunc);
}

// Video Focus
void RequestVideoFocus(FocusType eType, FocusPriority ePriority, bool *bOk)
{
	Frame *p = Frame::GetInstance();
	if (p)
		p->RequestVideoFocus(eType, ePriority, bOk);
}

void RegisterRequestVideoFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	Frame::RegisterRequestVideoFocus(cbFunc);
}

void RequestVideoFocusTransient(FocusPriority ePriority, bool *bOk)
{
	Frame *p = Frame::GetInstance();
	if (p)
		p->RequestVideoFocusTransient(ePriority,bOk);
}

void RegisterRequestVideoFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	Frame::RegisterRequestVideoFocusTransient(cbFunc);
}

void RegisterRequestVideoFocusLoss(void (*cbFunc)(void))
{
	Frame::RegisterRequestVideoFocusLoss(cbFunc);
}

void RegisterRequestTerminate(void (*cbFunc)())
{
	Frame::RegisterRequestTerminate(cbFunc);
}

void BackButtonClicked()
{
	Frame *p = Frame::GetInstance();
	if (p)
	{
		p->BackButtonClicked();
	}
}
