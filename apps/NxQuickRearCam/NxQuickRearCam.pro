#-------------------------------------------------
#
# Project created by QtCreator 2016-11-29T09:49:36
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NxQuickRearCam
TEMPLATE = app
QMAKE_RPATHDIR += /nexell/daudio/NxQuickRearCam/lib/

# Add RearCam Library
INCLUDEPATH += $$PWD/../../library/include
LIBS += -L$$PWD/../../library/lib -lnxrearcam
LIBS += -ldrm -lnx_v4l2 -lnx_video_api

# Add Common UI Module
LIBS += -L$$PWD/../../library/lib -lnxbaseui -lnxdaudioipc -lnxdaudioutils -lnxpacpclient
INCLUDEPATH += $$PWD/../../library/include

SOURCES += main.cpp\
	mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
