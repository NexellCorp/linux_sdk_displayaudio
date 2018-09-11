#ifndef NXEVENT_H
#define NXEVENT_H

#include <QEvent>

#define NX_BASE_EVENT_TYPE	(QEvent::User+100)

enum NxEventTypes {
	E_NX_EVENT_STATUS_HOME = NX_BASE_EVENT_TYPE,
	E_NX_EVENT_STATUS_BACK
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

#endif // NXEVENT_H
