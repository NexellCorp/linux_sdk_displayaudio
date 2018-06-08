#-------------------------------------------------
#
# Project created by QtCreator 2014-08-29T14:02:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = podo
TEMPLATE = app

linux* {
    lessThan(QT_MAJOR_VERSION, 5): LIBS += -lts
}

SOURCES += main.cpp \
    core.cpp

HEADERS += \
    core.h \
    global.h

INCLUDEPATH += ../pacp

target.path = $${PREFIX}/podo/core

#extra_files.path = $${PREFIX}/podo/core
#extra_files.files = run.sh

include(../pacp/pacp.pri))

INSTALLS += target
