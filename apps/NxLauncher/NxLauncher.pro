#-------------------------------------------------
#
# Project created by QtCreator 2017-05-02T11:54:15
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NxLauncher
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QT      += qml quickwidgets

greaterThan(QT_MINOR_VERSION, 6) {
    CONFIG += CONFIG_MEDIA_SCANNER
} else {
    CONFIG -= CONFIG_MEDIA_SCANNER
}

SOURCES += main.cpp\
#        nxlauncher.cpp \
    nxappinfo.cpp \
    nxpackagemanager.cpp \
    nxprocessmanager.cpp \
    NotificationFrame.cpp \
#    MessageWidget.cpp \
    NxLauncher.cpp \
    MessageFrame.cpp \
    ShadowEffect.cpp \
    CNX_VolumeBar.cpp

HEADERS  += \
    nxappinfo.h \
#nxlauncher.h \
    nxpackagemanager.h \
    nxprocessmanager.h \
    NxEvent.h \
    NotificationFrame.h \
#    MessageWidget.h \
    NxLauncher.h \
    MessageFrame.h \
    ShadowEffect.h \
    CNX_VolumeBar.h

FORMS    += \
#nxlauncher.ui \
    NotificationFrame.ui \
#    MessageWidget.ui \
    NxLauncher.ui \
    MessageFrame.ui \
    CNX_VolumeBar.ui

RESOURCES += \
    nxlauncher.qrc

CONFIG_MEDIA_SCANNER {
    DEFINES += CONFIG_MEDIA_SCANNER

    SOURCES += \
        media/CNX_UeventManager.cpp \
        media/CNX_MediaScanner.cpp \
        media/CNX_MediaDatabase.cpp \
        media/uevent.c \
        media/CNX_File.cpp \
        media/MediaScanner.cpp \
        media/CNX_VolumeManager.cpp \
        media/CNX_DiskManager.cpp

    HEADERS +=  \
        media/CNX_UeventManager.h \
        media/CNX_MediaScanner.h \
        media/CNX_MediaDatabase.h \
        media/uevent.h \
        media/CNX_Base.h \
        media/CNX_File.h \
        media/MediaScanner.h \
        media/CNX_VolumeManager.h \
        media/MediaConf.h \
        media/CNX_DiskManager.h
}

INCLUDEPATH += $$_PRO_FILE_PWD_/../../library/include
INCLUDEPATH += $$_PRO_FILE_PWD_/../../library/prebuilt/include

linux-oe-g++ {
    LIBS += -L$$_PRO_FILE_PWD_/../../library/lib -lnxkeyreceiver -lnxbaseui -lnxdaudioutils -lsqlite3 -lcrypto
} else {
    LIBS += -L$$_PRO_FILE_PWD_/../../library/lib/x64 -lnxkeyreceiver -lnxbaseui -lnxdaudioutils
}

# dynamic library
LIBS += -ldl

unix {
    target.path = $$_PRO_FILE_PWD_/../../bin/$${TARGET}
    INSTALLS += target
}

# conditional build option
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
