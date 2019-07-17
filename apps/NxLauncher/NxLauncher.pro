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

CONFIG += CONFIG_NXP4330

socname = $$getenv(OECORE_SOCNAME)
equals(socname, "") {
    message("OECORE_SOCNAME is empty")
} else {
    message($$socname)

    equals(socname, nxp3220) {
        CONFIG += CONFIG_NXP3220
        CONFIG -= CONFIG_NXP4330
    }
}

contains(CONFIG, CONFIG_NXP3220) {
    DEFINES += CONFIG_NXP3220
} else {
    DEFINES += CONFIG_NXP4330
    QT += qml quickwidgets
}

SOURCES += main.cpp\
    nxappinfo.cpp \
    nxpackagemanager.cpp \
    NotificationFrame.cpp \
    NxLauncher.cpp \
    MessageFrame.cpp \
    ShadowEffect.cpp \
    CNX_VolumeBar.cpp \
    media/CNX_UeventManager.cpp \
    media/CNX_MediaScanner.cpp \
    media/CNX_MediaDatabase.cpp \
    media/uevent.c \
    media/CNX_File.cpp \
    media/MediaScanner.cpp \
    media/CNX_VolumeManager.cpp \
    media/CNX_DiskManager.cpp \
    page/PageFrame.cpp \
    page/PageIndicatorFrame.cpp \
    page/PageItemFrame.cpp \
    page/PageStackFrame.cpp \
    InitThread.cpp

HEADERS  += \
    nxappinfo.h \
    nxpackagemanager.h \
    NxEvent.h \
    NotificationFrame.h \
    NxLauncher.h \
    MessageFrame.h \
    ShadowEffect.h \
    CNX_VolumeBar.h \
    media/CNX_UeventManager.h \
    media/CNX_MediaScanner.h \
    media/CNX_MediaDatabase.h \
    media/uevent.h \
    media/CNX_Base.h \
    media/CNX_File.h \
    media/MediaScanner.h \
    media/CNX_VolumeManager.h \
    media/MediaConf.h \
    media/CNX_DiskManager.h \
    page/PageFrame.h \
    page/PageIndicatorFrame.h \
    page/PageItemFrame.h \
    page/PageStackFrame.h \
    InitThread.h

FORMS    += \
    NotificationFrame.ui \
    NxLauncher.ui \
    MessageFrame.ui \
    CNX_VolumeBar.ui \
    page/PageFrame.ui \
    page/PageItemFrame.ui \
    page/PageStackFrame.ui

RESOURCES += \
    nxlauncher.qrc

INCLUDEPATH += $$_PRO_FILE_PWD_/../../library/include
INCLUDEPATH += $$_PRO_FILE_PWD_/../../library/prebuilt/include

linux-oe-g++ {
    LIBS += -L$$_PRO_FILE_PWD_/../../library/lib -lnxkeyreceiver -lnxbaseui -lnxdaudioutils -lsqlite3 -lcrypto -ludev
}

# dynamic library
LIBS += -ldl

unix {
    target.path = $$_PRO_FILE_PWD_/../../bin/$${TARGET}
    INSTALLS += target
}

# conditional build option
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
