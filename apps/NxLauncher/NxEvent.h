#ifndef NXEVENT_H
#define NXEVENT_H

#include <QEvent>
#include <QString>
#include <NX_Type.h>

#define NX_EVENT_BASE (QEvent::User+1)

enum NxEventTypes {
	E_NX_EVENT_KEY = NX_EVENT_BASE,
	E_NX_EVENT_POPUP_MESSAGE,
	E_NX_EVENT_NOTIFICATION,
	E_NX_EVENT_VOLUME_CONTROL,
	E_NX_EVENT_OPACITY,
	E_NX_EVENT_SDCARD_MOUNT,
	E_NX_EVENT_SDCARD_UMOUNT,
	E_NX_EVENT_SDCARD_INSERT,
	E_NX_EVENT_SDCARD_REMOVE,
	E_NX_EVENT_USB_INSERT,
	E_NX_EVENT_USB_REMOVE,
	E_NX_EVENT_USB_MOUNT,
	E_NX_EVENT_USB_UMOUNT,
	E_NX_EVENT_MEDIA_SCAN_DONE,
	E_NX_EVENT_TERMINATE
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

		for (int32_t i = ButtonType_Ok; i < ButtonType_Count; ++i)
		{
			if (psPopupMessage->pStylesheet[i])
				m_ButtonStylesheet[i] = psPopupMessage->pStylesheet[i];
		}

		m_eButtonVisibility = psPopupMessage->eVisibility;
		m_eButtonLocation = psPopupMessage->eLocation;
		m_uiTimeout = psPopupMessage->uiTimeout;
	}
};

class NxNotificationEvent : public QEvent
{
public:
	QString m_Requestor;

	QString m_MsgTitle;
	QString m_MsgBody;
	ButtonVisibility m_eButtonVisibility;
	QString m_ButtonStylesheet[ButtonType_Count];
	unsigned int m_uiTimeout;

	NxNotificationEvent(PopupMessage *psNotification, QString requestor)
		: QEvent((QEvent::Type)E_NX_EVENT_NOTIFICATION)
	{
		m_Requestor = requestor;

		if (psNotification->pMsgTitle)
			m_MsgTitle = QString::fromLatin1(psNotification->pMsgTitle);

		if (psNotification->pMsgBody)
			m_MsgBody = QString::fromLatin1(psNotification->pMsgBody);

		for (int32_t i = ButtonType_Ok; i < ButtonType_Count; ++i)
		{
			if (psNotification->pStylesheet[i])
				m_ButtonStylesheet[i] = psNotification->pStylesheet[i];
		}

		m_eButtonVisibility = psNotification->eVisibility;
		m_uiTimeout = psNotification->uiTimeout;
	}
};

class NxVolumeControlEvent : public QEvent
{
public:
	NxVolumeControlEvent() : QEvent((QEvent::Type)E_NX_EVENT_VOLUME_CONTROL)
	{

	}
};

class NxOpacityEvent : public QEvent
{
public:
	bool m_bOpacity;

	NxOpacityEvent(bool opacity) : QEvent((QEvent::Type)E_NX_EVENT_OPACITY)
	{
		m_bOpacity = opacity;
	}
};
#endif // NXEVENT_H
