#-------------------------------------------------
#
# Project created by QtCreator 2017-05-05T17:25:22
#
#-------------------------------------------------

QT       += widgets

TARGET = nxbaseui
TEMPLATE = lib

DEFINES += CNXBASEUI_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES +=  \
    CNX_StatusBar.cpp \
    CNX_BaseUI.cpp \
    CNX_MessageBox.cpp \
    CNX_KeyboardFrame.cpp \
    ElideLabel.cpp \
    CNX_LoadingBarWidget.cpp

HEADERS +=  \
    CNX_StatusBar.h \
    CNX_BaseUI.h \
    CNX_BaseUI_global.h \
    CNX_MessageBox.h \
    CNX_KeyboardFrame.h \
    ElideLabel.h \
    CNX_LoadingBarWidget.h

RESOURCES += \
    CNX_BaseUI.qrc \
    image/keyboard/Keyboard.qrc

unix {
    LIBS += -lsqlite3

    ###########################################################
    # daudio utils module
    LIBS += -L$$PWD/../../lib/ -lnxdaudioutils
    INCLUDEPATH += ../../include/

    header.path = $$_PRO_FILE_PWD_/../../include
    header.files = CNX_StatusBar.h CNX_MessageBox.h CNX_KeyboardFrame.h CNX_LoadingBarWidget.h

    target.path = $$_PRO_FILE_PWD_/../../lib
    INSTALLS += target header
}

# conditional build option
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

FORMS += \
    CNX_StatusBar.ui \
    CNX_MessageBox.ui \
    CNX_KeyboardFrame.ui

