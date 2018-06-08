#-------------------------------------------------
#
# Project created by QtCreator 2016-10-25T11:08:06
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

TARGET = NxVideoPlayer
TEMPLATE = app

# Add Graphic Tool Library
LIBS += -lMali -lnx_drm_allocator -lnx_video_api
LIBS += -L$$PWD/../../library/lib/ -lnxmpmanager -lnxfilterhelper -lnxfilter

equals(QT_VERSION, 5.7.0) {
      LIBS += -lnxsubtitleparser
}
else {
      LIBS += -lnxsubtitleparser_icui18n56
}

# Add SQL library
LIBS += -L$$PWD/../../library/lib/ -lnxdaudioutils

# Add IPC library
LIBS += -L$$PWD/../../library/lib/ -lnxdaudioipc -lnxpacpclient

# Add xml config library
LIBS += -L$$PWD/../../library/lib/ -lnxconfig -lxml2

# Add Common UI Module
LIBS += -L$$PWD/../../library/lib -lnxbaseui

INCLUDEPATH += $$PWD/../../library/include

INCLUDEPATH += \
    $$PWD/./vr_tools/include

SOURCES += main.cpp\
        mainwindow.cpp \
        qtglvideowindow.cpp \
        geometryengine.cpp \
        playlistwindow.cpp \
        CNX_MoviePlayer.cpp \
        CNX_FileList.cpp

HEADERS  += mainwindow.h \
        qtglvideowindow.h \
        geometryengine.h \
        CNX_Util.h \
        playlistwindow.h \
        CNX_Util.h \
        CNX_MoviePlayer.h \
        CNX_FileList.h

FORMS    += mainwindow.ui \
            playlistwindow.ui

RESOURCES += \
    shader.qrc
