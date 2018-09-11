#ifndef TIMEUTILITY_H
#define TIMEUTILITY_H

#ifndef SEC_PER_MIN
#   define SEC_PER_MIN     60
#endif

#ifndef MSEC_PER_SEC
#   define MSEC_PER_SEC    1000
#endif

#ifndef MSEC_PER_MIN
#   define MSEC_PER_MIN    (MSEC_PER_SEC*SEC_PER_MIN)
#endif

#ifndef MSEC_PER_HOUR
#   define MSEC_PER_HOUR   (MSEC_PER_MIN*SEC_PER_MIN)
#endif

#if defined(__using_NXBTUTIL__)
#ifdef QT_CORE_LIB
#include <QTime>
// moved from NxBTService (in DigitalAudio.svn)
void convertMiliSecToTime(qint32 msec, char *msecToTime) {
    QTime TimeInit = QTime(0, 0, 0, 0);
    QTime Time = TimeInit.addMSecs(msec);
    int hour = Time.hour();
    int min = Time.minute();
    int sec = Time.second();
    sprintf(msecToTime, "%02d:%02d:%02d", hour, min, sec);
#endif
}
#else /* #if !defined(__using_NXBTUTIL__) */
/*
 * To avoid Qt platform independent!
 */
#include <string>
using namespace std;

static void convertToRealtime(int msec, int* hour, int* minute, int* second)
{
    *hour = 0;
    *minute = 0;
    *second = 0;

    if (msec >= MSEC_PER_HOUR) {
        *hour = msec / MSEC_PER_HOUR;
        msec %= MSEC_PER_HOUR;
    }

    if (msec >= MSEC_PER_MIN) {
        *minute = msec / MSEC_PER_MIN;
        msec %= MSEC_PER_MIN;
    }

    if (msec >= MSEC_PER_SEC) {
        *second = msec / MSEC_PER_SEC;
    }
}

static void convertToRealtimeString(int msec, int *hour, int *minute, int *second, std::string* text)
{
    char buffer[100] = {0,};

    convertToRealtime(msec, hour, minute, second);

    if (*hour > 0) {
        sprintf(buffer, "%02d:", *hour);
    }

    sprintf(buffer+strlen(buffer), "%02d:%02d", *minute, *second);

    *text = (std::string)buffer;
}

static std::string convertToRealtimeString(int msec)
{
    int hour, minute, second;
    std::string rt;

    convertToRealtimeString(msec, &hour, &minute, &second, &rt);

    return rt;
}

#endif /* #if defined(__using_NXBTUTIL__) END */

#endif // TIMEUTILITY_H
