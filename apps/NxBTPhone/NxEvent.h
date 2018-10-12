#ifndef NXEVENT_H
#define NXEVENT_H

#include <QEvent>

#define NX_BASE_EVENT_TYPE	(QEvent::User+100)

enum NxEventTypes {
	E_NX_EVENT_STATUS_BACK = NX_BASE_EVENT_TYPE
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
