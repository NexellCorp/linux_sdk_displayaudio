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

DEFINES -= CONFIG_USE_NO_QML

equals(QT_VERSION, 5.7.0) {
} else {

}

SOURCES += main.cpp\
        nxlauncher.cpp \
    nxappinfo.cpp \
    nxpackagemanager.cpp \
    nxprocessmanager.cpp \
    page/PageFrame.cpp \
    page/PageItemFrame.cpp \
    page/PageStackFrame.cpp \
    page/PageIndicatorFrame.cpp

HEADERS  += nxlauncher.h \
    nxappinfo.h \
    nxpackagemanager.h \
    nxprocessmanager.h \
    page/PageFrame.h \
    page/PageItemFrame.h \
    page/PageStackFrame.h \
    page/PageIndicatorFrame.h

FORMS    += nxlauncher.ui \
    page/PageFrame.ui \
    page/PageItemFrame.ui \
    page/PageStackFrame.ui

RESOURCES += \
    nxlauncher.qrc

INCLUDEPATH += $$_PRO_FILE_PWD_/../../library/include
LIBS += -L$$_PRO_FILE_PWD_/../../library/lib -lnxbaseui -lnxdaudioipc -lnxkeyreceiver -lnxdaudioutils -lnxpacpclient

unix {
    target.path = $$_PRO_FILE_PWD_/../../bin/$${TARGET}
    INSTALLS += target
}

# conditional build option
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
