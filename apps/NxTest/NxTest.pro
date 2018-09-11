#-------------------------------------------------
#
# Project created by QtCreator 2018-09-05T15:04:27
#
#-------------------------------------------------

QT       += widgets core gui

TARGET = nxtest
TEMPLATE = lib

DEFINES += LIBNXTEST_LIBRARY

CONFIG += plugin
CONFIG -= TEST2

contains(CONFIG, TEST2) {
    message("test2")
    TARGET = nxtest2
    DEFINES += CONFIG_TEST2
} else {
    message("test1")
    TARGET = nxtest1
}

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
#    Frame.cpp \
    Form.cpp \
    DAudioIface_Impl.cpp


HEADERS += \
        libnxtest_global.h \ 
#    Frame.h \
#    DAudioIface.h \
    Form.h \
    NxEvent.h

FORMS += \
    Form.ui
#    Frame.ui \

unix {
    target.path = /usr/lib
    INSTALLS += target
}



INCLUDEPATH += $$_PRO_FILE_PWD_/../../library/include
LIBS += -L$$_PRO_FILE_PWD_/../../library/lib -lnxbaseui
