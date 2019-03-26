#-------------------------------------------------
#
# Project created by QtCreator 2018-09-06T15:57:50
#
#-------------------------------------------------

QT       += widgets

TARGET = nxbtsettings
TEMPLATE = lib

DEFINES += LIBNXBTSETTINGS_LIBRARY

CONFIG += plugin

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
    MainFrame.cpp \
    SelectMenuFrame.cpp \
    ConnectionMenuFrame.cpp \
    AdvancedOptionFrame.cpp \
    BTCommandProcessor.cpp \
    BTPairedDeviceItem.cpp \
    Keyboard/KeyboardFrame.cpp \
    DAudioIface_Impl.cpp

HEADERS += \
    MainFrame.h \
    SelectMenuFrame.h \
    Types.h \
    ConnectionMenuFrame.h \
    AdvancedOptionFrame.h \
    BTCommandProcessor.h \
    BTPairedDeviceItem.h \
    Keyboard/KeyboardFrame.h \
    NxEvent.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    MainFrame.ui \
    SelectMenuFrame.ui \
    ConnectionMenuFrame.ui \
    AdvancedOptionFrame.ui \
    BTPairedDeviceItem.ui \
    Keyboard/KeyboardFrame.ui


linux-oe-g++ {
    INCLUDEPATH += $$PWD/../../library/include
    INCLUDEPATH += $$PWD/../../library/prebuilt/include
    LIBS += -L$$PWD/../../library/lib -lnxbaseui -lnxdaudioutils
} else {
    INCLUDEPATH += $$PWD/../../library/include
    INCLUDEPATH += $$PWD/../../library/prebuilt/include
    LIBS += -L$$PWD/../../library/lib/x64 -lnxbaseui -lnxdaudioutils
}
