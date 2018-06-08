#-------------------------------------------------
#
# Project created by QtCreator 2017-12-20T20:09:52
#
#-------------------------------------------------

equals(QT_VERSION, 5.7.0) {
QT       -= gui
} else {
QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
}

TARGET = nxpacpclient
TEMPLATE = lib

DEFINES += LIBNXPACPCLIENT_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

equals(QT_VERSION, 5.7.0) {
	include(pacp/pacp.pri)
	INCLUDEPATH += pacp
} else {
	DEFINES += QT_X11
}

INCLUDEPATH += ../../include/

SOURCES +=	\
	CNX_PacpClient.cpp		\
	NX_PacpClient.cpp

HEADERS +=	\
    libnxpacpclient_global.h\
    CNX_PacpClient.h		\
    NX_PacpClient.h

unix {
    header.path = $$_PRO_FILE_PWD_/../../include
    header.files = NX_PacpClient.h

    target.path = $$_PRO_FILE_PWD_/../../lib
    INSTALLS += target header
}

# conditional build option
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
