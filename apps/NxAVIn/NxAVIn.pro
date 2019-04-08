#-------------------------------------------------
#
# Project created by QtCreator 2017-09-26T17:01:23
#
#-------------------------------------------------

QT       += core gui
QT       += multimedia  \
            multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NxAVIn
TEMPLATE = lib
CONFIG += plugin

# Add Graphic tool libraries
LIBS += -lnx_drm_allocator -lnx_video_api
LIBS += -L$$PWD/../../library/lib -lnxavin

LIBS += -L$$PWD/../../library/lib -lnxdaudioutils

# Add xml config library
LIBS += -L$$PWD/../../library/lib/ -lnx_config -lxml2

# Add Common UI Module
LIBS += -L$$PWD/../../library/lib -lnxbaseui

INCLUDEPATH += $$PWD/../../library/include
INCLUDEPATH += $$PWD/../../library/prebuilt/include

SOURCES += \
    MainFrame.cpp \
    DAudioIface_Impl.cpp \
    AVInFrame.cpp \

HEADERS  += \
    MainFrame.h \
    NxEvent.h \
    AVInFrame.h \

FORMS    += \
    MainFrame.ui \
    AVInFrame.ui \
