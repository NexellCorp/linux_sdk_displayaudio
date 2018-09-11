#ifndef NXEVENT_H
#define NXEVENT_H

#include <QEvent>
#include <QString>
#include <NX_Type.h>

#define NX_EVENT_BASE (QEvent::User+1)

enum NxEventTypes {
	E_NX_EVENT_KEY,
	E_NX_EVENT_POPUP_MESSAGE
};

class NxKeyEvent : public QEvent
{
public:
	int32_t m_iKey;

	NxKeyEvent(int32_t iKey)
		: QEvent((QEvent::Type)E_NX_EVENT_KEY)
	{
		m_iKey = iKey;
	}
};

class NxPopupMessageEvent : public QEvent
{
public:
	QString m_Requestor;

	QString m_MsgTitle;
	QString m_MsgBody;
	ButtonVisibility m_eButtonVisibility;
	ButtonLocation m_eButtonLocation;
	QString m_ButtonStylesheet[ButtonType_Count];
	unsigned int m_uiTimeout;

	NxPopupMessageEvent(PopupMessage *psPopupMessage, QString requestor)
		: QEvent((QEvent::Type)E_NX_EVENT_POPUP_MESSAGE)
	{
		m_Requestor = requestor;

		if (psPopupMessage->pMsgTitle)
			m_MsgTitle = QString::fromLatin1(psPopupMessage->pMsgTitle);

		if (psPopupMessage->pMsgBody)
			m_MsgBody = QString::fromLatin1(psPopupMessage->pMsgBody);

		for (int32_t i = 0; i < ButtonType_Count; ++i)
		{
			if (psPopupMessage->pStylesheet[i])
				m_ButtonStylesheet[i] = psPopupMessage->pStylesheet[i];
		}

		m_eButtonVisibility = psPopupMessage->eVisibility;
		m_eButtonLocation = psPopupMessage->eLocation;
		m_uiTimeout = psPopupMessage->uiTimeout;
	}
};

#endif // NXEVENT_H
