#ifndef NXEVENT_H
#define NXEVENT_H

#ifdef QT_CORE_LIB
#include <QEvent>

class NxEvent : QEvent
{
public:
    int32_t m_iEventType;
    QString m_szData;

    NxEvent(QEvent::Type type) : QEvent(type)
    {
    }
};
#endif

#endif // NXEVENT_H
