#-------------------------------------------------
#
# Project created by QtCreator 2017-07-10T15:18:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CarBtnEmulator
TEMPLATE = app

INCLUDEPATH += $$PWD/../../library/src/KeyReceiver/

SOURCES += main.cpp\
	CarBtnWindow.cpp \
	TabSetting.cpp \
	TabButtons.cpp \
	CSettings.cpp \
	../../library/src/KeyReceiver/DAudioKeyDef.cpp \
	TabTest.cpp

HEADERS  += CarBtnWindow.h \
	TabSetting.h \
	TabButtons.h \
	CSettings.h \
	../../library/src/KeyReceiver/DAudioKeyDef.h \
    TabTest.h

FORMS += CarBtnWindow.ui \
	TabSetting.ui \
	TabButtons.ui \
	TabTest.ui
