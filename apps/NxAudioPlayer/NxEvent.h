#ifndef NXEVENT_H
#define NXEVENT_H

#include <QEvent>

#define NX_BASE_EVENT_TYPE	(QEvent::User+100)

enum NxEventTypes {
	E_NX_EVENT_STATUS_HOME = NX_BASE_EVENT_TYPE,
	E_NX_EVENT_STATUS_BACK,
	E_NX_EVENT_STATUS_VOLUME,
	E_NX_EVENT_TERMINATE,
	E_NX_EVENT_UPDATE_ALBUMART
};

class NxStatusHomeEvent : public QEvent
{
public:
	NxStatusHomeEvent() :
		QEvent((QEvent::Type)E_NX_EVENT_STATUS_HOME)
	{

	}
};

class NxStatusBackEvent : public QEvent
{
public:
	NxStatusBackEvent() :
		QEvent((QEvent::Type)E_NX_EVENT_STATUS_BACK)
	{

	}
};

class NxStatusVolumeEvent : public QEvent
{
public:
	NxStatusVolumeEvent() :
		QEvent((QEvent::Type)E_NX_EVENT_STATUS_VOLUME)
	{

	}
};

class NxTerminateEvent : public QEvent
{
public:
	NxTerminateEvent() :
		QEvent((QEvent::Type)E_NX_EVENT_TERMINATE)
	{

	}
};

class NxAlbumartUpdateEvent : public QEvent
{
public:
	QString m_Albumart;

	NxAlbumartUpdateEvent(QString path) :
		QEvent((QEvent::Type)E_NX_EVENT_UPDATE_ALBUMART)
	{
		m_Albumart = path;
	}
};

#endif // NXEVENT_H
