#ifndef NXEVENT_H
#define NXEVENT_H

#include <QEvent>

#define NX_BASE_EVENT_TYPE	(QEvent::User+100)

enum NxEventTypes {
	E_NX_EVENT_TERMINATE = NX_BASE_EVENT_TYPE
};

class NxTerminateEvent : public QEvent
{
public:
	NxTerminateEvent() :
		QEvent((QEvent::Type)E_NX_EVENT_TERMINATE)
	{

	}
};

#endif // NXEVENT_H
