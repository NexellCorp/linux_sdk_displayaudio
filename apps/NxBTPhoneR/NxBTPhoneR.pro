#-------------------------------------------------
#
# Project created by QtCreator 2017-10-30T08:13:37
#
#-------------------------------------------------

QT       += core gui
QT       += quickwidgets # for status bar (NxBaseUi library)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NxBTPhoneR
TEMPLATE = app

CONFIG += NxDAudioManager
NxDAudioManager {
    DEFINES += CONFIG_NX_DAUDIO_MANAGER
    DEFINES += CONFIG_USE_AUDIO_FOCUS
}

DEFINES += CONFIG_TEST_FLAG

DEFINES -= __dmesg__

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
    BTCommandProcessor.cpp \
    UDS_Client.cpp \
    InCommingCallDialog.cpp \
    io/vCard/VCardReader.cpp \
    gui/phonebook/PhoneBookItem.cpp \
    gui/CallLog/CallLogItem.cpp \
    CallMenuWidget.cpp \
    MessageMenuWidget.cpp \
    SelectMenuWidget.cpp \
    MainDialog.cpp

HEADERS += \
    BTCommandProcessor.h \
    UDS_Client.h \
    LogUtility.hpp \
    InCommingCallDialog.h \
    io/vCard/VCardReader.h \
    gui/phonebook/PhoneBookItem.h \
    gui/CallLog/CallLogItem.h \
    defines.h \
    CallMenuWidget.h \
    MessageMenuWidget.h \
    SelectMenuWidget.h \
    MainDialog.h \
    NxEvent.h

FORMS += \
    InCommingCallDialog.ui \
    gui/CallLog/CallLogItem.ui \
    CallMenuWidget.ui \
    SelectMenuWidget.ui \
    MessageMenuWidget.ui \
    MainDialog.ui

RESOURCES += \
    UI/Keyboard.qrc \
    UI/NxBTPhoneR.qrc

DISTFILES += \
    UI/bs.png \
    UI/bs_pressed.png \
    UI/shift.png \
    UI/shift_pressed.png \
    UI/loading/tenor.gif

TRANSLATIONS += lang_ko.ts

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

    LIBS += -lpthread
}
