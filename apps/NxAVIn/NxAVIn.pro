#-------------------------------------------------
#
# Project created by QtCreator 2017-09-26T17:01:23
#
#-------------------------------------------------

QT       += core gui \
            quickwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NxAVIn
TEMPLATE = app
QMAKE_RPATHDIR += /nexell/daudio/NxAVIn/lib/

# Add AVIN Library
INCLUDEPATH += $$PWD/../../library/include
LIBS += -L$$PWD/../../library/lib -lnxavin
LIBS += -ldrm -lnx_v4l2 -lnx_video_api

# Add Common UI Module
LIBS += -L$$PWD/../../library/lib -lnxbaseui -lnxdaudioipc -lnxdaudioutils -lnxpacpclient
INCLUDEPATH += $$PWD/../../library/include

SOURCES += main.cpp\
        avinmainwindow.cpp \
    avindisplayview.cpp

HEADERS  += avinmainwindow.h \
    ../../library/include/NX_AVIn.h \
    avindisplayview.h

FORMS    += avinmainwindow.ui
