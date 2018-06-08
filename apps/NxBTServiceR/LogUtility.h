#ifndef LOGUTILITY_H
#define LOGUTILITY_H

#if __dmesg__

#   define LOG(X)  do {                 \
        fprintf(stderr, "%s\n", X);     \
    } while(0)

#   ifdef QT_CORE_LIB
#       define LOGQ(X) do {     \
            qDebug() << X;      \
        } while (0)
#   endif /* QT_CORE_LIB */

#   ifdef __timestamp__
#       include <sys/time.h>
#       define LOGT(X) do {                                                             \
            struct timeval tv;                                                          \
            char datetime[1024] = {0,};                                                 \
            char buffer[1024];                                                          \
            gettimeofday(&tv, NULL);                                                    \
            strftime(datetime, 80, "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));         \
                                                                                        \
            sprintf(buffer, "[%s.%ld] %s", datetime, tv.tv_usec / 1000, X);     \
                                                                                        \
            LOG(buffer);                                                                \
        } while (0)
#   else
#      define LOGT(X) do { } while(0)
#   endif /* __timestamp__ */
#else /* !__dmesg__ */
#   define LOG(X)  do { } while(0)
#   define LOGQ(X) do { } while(0)
#   define LOGT(X) do { } while(0)
#endif

#endif // LOGUTILITY_H
