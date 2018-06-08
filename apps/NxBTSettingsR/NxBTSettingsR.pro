#-------------------------------------------------
#
# Project created by QtCreator 2017-10-24T19:04:51
#
#-------------------------------------------------

QT       += core gui
QT       += quickwidgets # for status bar (NxBaseUi library)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NxBTSettingsR
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
    UDS_Client.cpp \
    BTCommandProcessor.cpp \
    MessageDialog.cpp \
    BTPairedDeviceItem.cpp \
    MainDialog.cpp \
    SelectMenuWidget.cpp \
    ConnectionMenuWidget.cpp \
    AdvancedMenuWidget.cpp \
    Keyboard/KeyboardFrame.cpp \
    Keyboard/KeyboardDialog.cpp

HEADERS += \
    UDS_Client.h \
    BTCommandProcessor.h \
    MessageDialog.h \
    BTPairedDeviceItem.h \
    LogUtility.hpp \
    defines.h \
    MainDialog.h \
    SelectMenuWidget.h \
    ConnectionMenuWidget.h \
    AdvancedMenuWidget.h \
    Keyboard/KeyboardFrame.h \
    Keyboard/KeyboardDialog.h

FORMS += \
    MessageDialog.ui \
    BTPairedDeviceItem.ui \
    MainDialog.ui \
    SelectMenuWidget.ui \
    ConnectionMenuWidget.ui \
    AdvancedMenuWidget.ui \
    Keyboard/KeyboardFrame.ui \
    Keyboard/KeyboardDialog.ui

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

RESOURCES += \
    resource.qrc
