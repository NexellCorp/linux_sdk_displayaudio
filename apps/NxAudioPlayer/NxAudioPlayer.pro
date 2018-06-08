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
TEMPLATE = app

QMAKE_RPATHDIR += /nexell/daudio/NxAudioPlayer/lib/

# Add Media Player's Libraries
LIBS += -lnx_drm_allocator -lnx_video_api
LIBS += -L$$PWD/../../library/lib/ -lnxmpmanager -lnxfilterhelper -lnxfilter

equals(QT_VERSION, 5.7.0) {
      LIBS += -lnxsubtitleparser
}
else {
      LIBS += -lnxsubtitleparser_icui18n56
}

INCLUDEPATH += $$PWD/../../library/include

# Add SQL library
LIBS += -L$$PWD/../../library/lib/ -lnxdaudioutils

# Add IPC library
LIBS += -L$$PWD/../../library/lib/ -lnxdaudioipc -lnxpacpclient

# Add xml config library
LIBS += -L$$PWD/../../library/lib/ -lnxconfig -lxml2

# Add Common UI Module
LIBS += -L$$PWD/../../library/lib -lnxbaseui

# Add id3 library.
INCLUDEPATH += $$PWD/../../library/lib/id3-3.8/include
LIBS        += $$PWD/../../library/lib/id3-3.8/lib/libid3.so

SOURCES += main.cpp \
            mainwindow.cpp \
            playlistwindow.cpp \
            CNX_FileList.cpp \
            CNX_MoviePlayer.cpp

HEADERS  += mainwindow.h \
            playlistwindow.h \
            ../../library/include/NX_MoviePlay.h \
            ../../library/include/NX_Utils.h \
            CAudioPlayerSignals.h \
            CNX_FileList.h \
            CNX_MoviePlayer.h

FORMS    += mainwindow.ui \
            playlistwindow.ui

DISTFILES += \
            default.jpeg

RESOURCES += \
	resources.qrc
