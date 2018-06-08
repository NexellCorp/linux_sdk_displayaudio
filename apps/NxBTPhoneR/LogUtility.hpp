#ifndef LOGUTILITY_H
#define LOGUTILITY_H

#if __dmesg__
#define STR(X)  #X

#define LOG(X)  do {                \
        fprintf(stderr, "%s\n", X); \
    } while(0)

#ifdef QT_CORE_LIB
#include <QDebug>
#include <QDateTime>
#include <QApplication>

#define LOGQ(X) do {                \
        qDebug() << X;              \
    } while (0)

#define LOGQT(X) do {                                                                   \
        LOGQ(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << X);    \
    } while (0)

#define LOGQTA(X) do {                                                                                                      \
        LOGQ(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << QApplication::applicationName() << X);     \
    } while (0)

#endif  /* QT_CORE_LIB */
#else /* !__dmesg__ */
#define STR(X)
#define LOG(X)  do { } while(0)
#define LOGQ(X) do { } while(0)
#define LOGQT(X) do { } while(0)
#define LOGQTA(X) do { } while(0)
#endif /* __dmesg__ */

#endif // LOGUTILITY_H
