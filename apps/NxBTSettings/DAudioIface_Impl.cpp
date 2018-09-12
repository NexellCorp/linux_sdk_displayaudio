#include <DAudioIface.h>
#include "MainFrame.h"

#define LOG_TAG "[NxBTSettings]"
#include <NX_Log.h>

#include <QDebug>

void Init(void *pObj, const char *pArgs)
{
	(void)pArgs;
	MainFrame *p = MainFrame::GetInstance(pObj);
	if (p)
		p->Initialize();
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
void SendMessage(const char *pMsg, int32_t iMsgSize)
{
	MainFrame *p = MainFrame::GetInstance();
	if (p)
		p->SendMessage(pMsg);

	(void)iMsgSize;
}

void RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize))
{
	MainFrame::RegisterRequestSendMessage(cbFunc);
}

void RegisterRequestPopupMessage(void (*cbFunc)(PopupMessage *, bool *))
{
	MainFrame::RegisterRequestPopupMessage(cbFunc);
}

void PopupMessageResponse(bool bOk)
{
	MainFrame *p = MainFrame::GetInstance();
	if (p)
		p->PopupMessageResponse(bOk);
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

//
void RegisterRequestTerminate(void (*cbFunc)())
{
	MainFrame::RegisterRequestTerminate(cbFunc);
}
