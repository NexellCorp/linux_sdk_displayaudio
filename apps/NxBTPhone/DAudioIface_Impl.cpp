#include <DAudioIface.h>
#include "MainFrame.h"

#define LOG_TAG "[NxBTAudio]"
#include <NX_Log.h>

void Init(void *pObj, const char *pArgs)
{
	size_t len = 0;
	MainFrame *p = MainFrame::GetInstance(pObj);
	if (!p)
	{
		NXLOGE("[%s] Instance does not exists.", __FUNCTION__);
		return;
	}

	if (!p->Initialize(pArgs))
	{
		NXLOGE("[%s] Initialize().", __FUNCTION__);
		return;
	}
}

void IsInit(bool *bOk)
{
	*bOk = MainFrame::GetInstance() ? true : false;
}

void deInit()
{
	MainFrame::DestroyInstance();
}

void Show()
{
	NXLOGI("[%s]", __FUNCTION__);
	MainFrame *p = MainFrame::GetInstance();
	if (p)
		p->show();
}

void Hide()
{
	NXLOGI("[%s]", __FUNCTION__);
	MainFrame *p = MainFrame::GetInstance();
	if (p)
		p->hide();
}

void Raise()
{
	NXLOGI("[%s]", __FUNCTION__);
	MainFrame *p = MainFrame::GetInstance();
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
	MainFrame *p = MainFrame::GetInstance();
	if (p)
		p->lower();
}

// Launcher Show
void RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk))
{
	MainFrame::RegisterRequestLauncherShow(cbFunc);
}

// Message
void SendMessage(const char *pSrc, const char *pMsg, int32_t iMsgSize)
{
	MainFrame *p = MainFrame::GetInstance();
	if (p)
		p->SendMessage(pMsg);
}

void RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize))
{
	MainFrame::RegisterRequestSendMessage(cbFunc);
}

// Video Focus
void RequestVideoFocus(FocusType eType, FocusPriority ePriority, bool *bOk)
{
	MainFrame *p = MainFrame::GetInstance();
	if (p)
		p->RequestVideoFocus(eType, ePriority, bOk);
}

void RegisterRequestVideoFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	MainFrame::RegisterRequestVideoFocus(cbFunc);
}

void RequestVideoFocusTransient(FocusPriority ePriority, bool *bOk)
{
	MainFrame *p = MainFrame::GetInstance();
	if (p)
		p->RequestVideoFocusTransient(ePriority,bOk);
}

void RegisterRequestVideoFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	MainFrame::RegisterRequestVideoFocusTransient(cbFunc);
}

void RegisterRequestVideoFocusLoss(void (*cbFunc)(void))
{
	MainFrame::RegisterRequestVideoFocusLoss(cbFunc);
}

void RegisterRequestTerminate(void (*cbFunc)())
{
	MainFrame::RegisterRequestTerminate(cbFunc);
}
