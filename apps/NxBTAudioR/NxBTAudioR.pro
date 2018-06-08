#-------------------------------------------------
#
# Project created by QtCreator 2017-10-27T07:58:18
#
#-------------------------------------------------

QT       += core gui
QT       += quickwidgets # for status bar (NxBaseUi library)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NxBTAudioR
TEMPLATE = app

CONFIG += NxDAudioManager
NxDAudioManager {
    DEFINES += CONFIG_NX_DAUDIO_MANAGER
}

DEFINES += CONFIG_TEST_FLAG

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        Dialog.cpp \
    BTCommandProcessor.cpp \
    UDS_Client.cpp

HEADERS += \
        Dialog.h \
    BTCommandProcessor.h \
    UDS_Client.h \
    TimeUtility.hpp \
    LogUtility.hpp

FORMS += \
        Dialog.ui

RESOURCES += \
    Images.qrc

linux-oe-g++ {
    ###########################################################
    #  Set DisplayAudio library include and library path
    NX_DAUDIO_LIB_PATH = $$PWD/../../library

    INCLUDEPATH += $${NX_DAUDIO_LIB_PATH}/include/
    LIBS += -L$${NX_DAUDIO_LIB_PATH}/lib/

    ###########################################################
    # ipc module
    LIBS += -lnxdaudioipc -lnxpacpclient

    ###########################################################
    # ui module (for status bar)
    LIBS += -lnxbaseui -lnxdaudioutils
} else {
    error("current toolchain is not supported!")
}
