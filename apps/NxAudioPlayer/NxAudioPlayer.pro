#-------------------------------------------------
#
# Project created by QtCreator 2016-09-27T09:40:24
#
#-------------------------------------------------

QT += core gui

QT += network \
      xml \
      multimedia \
      multimediawidgets \
      widgets \
      quickwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NxAudioPlayer
TEMPLATE = lib
CONFIG += plugin

# Add Media Player Libraries
LIBS += -lnx_drm_allocator -lnx_video_api
LIBS += -L$$PWD/../../library/prebuilt/lib -lnxmpmanager -lnxfilterhelper -lnxfilter

INCLUDEPATH += $$PWD/../../library/include
INCLUDEPATH += $$PWD/../../library/prebuilt/include

# Add SQL library
LIBS += -L$$PWD/../../library/lib/ -lnxdaudioutils

# Add xml config library
LIBS += -L$$PWD/../../library/lib/ -lnxconfig -lxml2

# Add Common UI Module
LIBS += -L$$PWD/../../library/lib -lnxbaseui

# Add Id3 libraries
LIBS += -lid3 -lid3tag

SOURCES +=   \
    CNX_FileList.cpp \
    CNX_AudioPlayer.cpp \
    MainFrame.cpp \
    DAudioIface_Impl.cpp \
    PlayListAudioFrame.cpp \
    PlayerAudioFrame.cpp

HEADERS += \
    ../../library/prebuilt/include/NX_MoviePlay.h \
    ../../library/prebuilt/include/NX_Utils.h \
    CAudioPlayerSignals.h \
    CNX_FileList.h \
    CNX_AudioPlayer.h \
    MainFrame.h \
    PlayListAudioFrame.h \
    PlayerAudioFrame.h \
    NxEvent.h

FORMS += \
    MainFrame.ui \
    PlayerAudioFrame.ui \
    PlayListAudioFrame.ui

DISTFILES += \
    default.jpeg

RESOURCES += \
    resources.qrc
