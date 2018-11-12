#-------------------------------------------------
#
# Project created by QtCreator 2016-11-29T09:49:36
#
#-------------------------------------------------

QT       += core gui
QT       += network     \
            xml         \
            multimedia  \
            multimediawidgets \
            widgets \
            quickwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NxQuickRearCam
TEMPLATE = lib
CONFIG += plugin

# Add Graphic tool libraries
LIBS += -lnx_drm_allocator -lnx_video_api
LIBS += -L$$PWD/../../library/prebuilt/lib -lnxrearcam

LIBS += -L$$PWD/../../library/lib -lnxdaudioutils

# Add xml config library
LIBS += -L$$PWD/../../library/lib/ -lnxconfig -lxml2

# Add Common UI Module
LIBS += -L$$PWD/../../library/lib -lnxbaseui

INCLUDEPATH += $$PWD/../../library/include
INCLUDEPATH += $$PWD/../../library/prebuilt/include

SOURCES += \
    MainFrame.cpp \
    DAudioIface_Impl.cpp \
    QuickRearCamFrame.cpp \

HEADERS  += \
    MainFrame.h \
    NxEvent.h \
    QuickRearCamFrame.h \

FORMS    += \
    MainFrame.ui \
    QuickRearCamFrame.ui \
