#-------------------------------------------------
#
# Project created by QtCreator 2016-10-25T11:08:06
#
#-------------------------------------------------

QT       += core gui
QT       += multimedia  \
            multimediawidgets \

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NxVideoPlayer
TEMPLATE = lib
CONFIG += plugin

    # Add Graphic tool libraries
    LIBS += -lnx_video_api
    LIBS += -L$$PWD/../../library/prebuilt/lib -lnxmpmanager -lnxfilterhelper -lnxfilter

    # Add SQL library
    LIBS += -L$$PWD/../../library/lib -lnxdaudioutils

    # Add xml config library
    LIBS += -L$$PWD/../../library/lib -lnx_config -lxml2

    # Add Common UI Module
    LIBS += -L$$PWD/../../library/lib -lnxbaseui

    # Add icu libraries
    LIBS += -licuuc -licui18n

    INCLUDEPATH += $$PWD/../../library/include
    INCLUDEPATH += $$PWD/../../library/prebuilt/include

SOURCES += \
    CNX_MoviePlayer.cpp \
    CNX_FileList.cpp \
    CNX_SubtitleParser.cpp \
    MainFrame.cpp \
    DAudioIface_Impl.cpp \
    PlayerVideoFrame.cpp \
    PlayListVideoFrame.cpp

HEADERS  += \
    CNX_Util.h \
    CNX_Util.h \
    CNX_MoviePlayer.h \
    CNX_FileList.h \
    CNX_SubtitleParser.h \
    MainFrame.h \
    NxEvent.h \
    PlayListVideoFrame.h \
    PlayerVideoFrame.h

FORMS    += \
    MainFrame.ui \
    PlayerVideoFrame.ui \
    PlayListVideoFrame.ui
