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

equals(QT_VERSION, 5.7.0) {
} else {

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

INCLUDEPATH += $$_PRO_FILE_PWD_/../../library/include

#LIBS += -L$$_PRO_FILE_PWD_/../../library/lib -lnxbaseui -lnxdaudioipc -lnxkeyreceiver -lnxdaudioutils -lnxpacpclient
linux-oe-g++ {
    LIBS += -L$$_PRO_FILE_PWD_/../../library/lib -lnxkeyreceiver -lnxbaseui -lnxdaudioutils
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
