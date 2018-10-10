#include <DAudioIface.h>
#include "NxBTService.h"

#define LOG_TAG "[NxBTService]"
#include <NX_Log.h>

/************************************************************************************\
 * D-AUDIO INTERFACE - Init
 *
 * Description
 *  -
 ************************************************************************************/
void Init(void *pObj, const char *pArgs)
{
	NxBTService *p = NxBTService::GetInstance(pObj);
	if (p) {
		p->Initialize();
	}
}

/************************************************************************************\
 * D-AUDIO INTERFACE - IsInit
 *
 * Description
 *  -
 ************************************************************************************/
void IsInit(bool *bOk)
{
	*bOk = NxBTService::GetInstance() ? true : false;
}

/************************************************************************************\
 * D-AUDIO INTERFACE - deInit
 *
 * Description
 *  -
 ************************************************************************************/
void deInit()
{
	NxBTService::DestroyInstance();
}

/************************************************************************************\
 * D-AUDIO INTERFACE - SendMessage
 *
 * Description
 *  - Receive message from Launcher
 ************************************************************************************/
void SendMessage(const char *pSrc, const char *pMsg, int32_t iMsgSize)
{
	NxBTService *p = NxBTService::GetInstance();
	if (p) {
		p->runCommand(pMsg);
	}
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RegisterRequestSendMessage
 *
 * Description
 *  - Register callback function for 'RequestSendMessage'
 *  - RequestSendMessage sends the message to the destination via the launcher.
 *
 * Arguments
 *  - cbFunc : function pointer of callback
 *  - pDst   : destination
 *  - pMsg   : message
 ************************************************************************************/
void RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize))
{
	NxBTService::RegisterRequestSendMessage(cbFunc);
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RegisterRequestPopupMessage
 *
 * Description
 *  -
 ************************************************************************************/
void RegisterRequestPopupMessage(void (*cbFunc)(PopupMessage *, bool *))
{
	NxBTService::RegisterRequestPopupMessage(cbFunc);
}

void RegisterRequestExpirePopupMessage(void (*cbFunc)())
{
	NxBTService::RegisterRequestExpirePopupMessage(cbFunc);
}

/************************************************************************************\
 * D-AUDIO INTERFACE - PopupMessageResponse
 *
 * Description
 *  -
 ************************************************************************************/
void PopupMessageResponse(bool bOk)
{
	NxBTService *p = NxBTService::GetInstance();
	if (p) {
		p->PopupMessageResponse(bOk);
	}
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestAudioFocus
 *
 * Description
 *  - When this function is called, it should return audio focus if possible.
 *  - However, it does not return audio focus when calling on a BT phone.
 *
 * Argument
 *  - bOk : This is the reseult of release audio focus.
 ************************************************************************************/
void RequestAudioFocus(FocusType eType, FocusPriority ePriority, bool *bOk)
{
	NxBTService *p = NxBTService::GetInstance();
	if (p) {
		p->RequestAudioFocus(eType, ePriority, bOk);
	}
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RegisterRequestAudioFocus
 *
 * Description
 *  - Register callback function for 'RequestAudioFocus'
 *  - It is a command to get audio focus.
 ************************************************************************************/
void RegisterRequestAudioFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	NxBTService::RegisterRequestAudioFocus(cbFunc);
}

/************************************************************************************\
 * D-AUDIO INTERFACE -
 *
 * Description
 *  -
 ************************************************************************************/
void RequestAudioFocusTransient(FocusPriority ePriority, bool *bOk)
{
	NxBTService *p = NxBTService::GetInstance();
	if (p) {
		p->RequestAudioFocusTransient(ePriority, bOk);
	}
}

/************************************************************************************\
 * D-AUDIO INTERFACE -
 *
 * Description
 *  -
 ************************************************************************************/
void RegisterRequestAudioFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	NxBTService::RegisterRequestAudioFocusTransient(cbFunc);
}

/************************************************************************************\
 * D-AUDIO INTERFACE -
 *
 * Description
 *  -
 ************************************************************************************/
void RegisterRequestAudioFocusLoss(void (*cbFunc)(void))
{
	NxBTService::RegisterRequestAudioFocusLoss(cbFunc);
}

/************************************************************************************\
 * D-AUDIO INTERFACE -
 *
 * Description
 *  -
 ************************************************************************************/
void RegisterRequestPlugInRun(void (*cbFunc)(const char *pPlugin, const char *pArgs))
{
	NxBTService::RegisterRequestPlugInRun(cbFunc);
}

/************************************************************************************\
 * D-AUDIO INTERFACE -
 *
 * Description
 *  -
 ************************************************************************************/
void RegisterRequestPlugInTerminate(void (*cbFunc)(const char *pPlugin))
{
	NxBTService::RegisterRequestPlugInTerminate(cbFunc);
}
