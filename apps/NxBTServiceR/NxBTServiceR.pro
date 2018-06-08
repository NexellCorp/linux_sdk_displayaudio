TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    NxBTService.cpp \
    UDS_Server.cpp

HEADERS += \
    NxBTService.h \
    UDS_Server.h \
    LogUtility.h \
    ExceptionHandler.h

# if specific toolchain ( if current toolchain is 'd-audio system toolchain', assume condition 'true'.)
linux-oe-g++ {
    # Test Options
    DEFINES -= __dmesg__
    DEFINES -= __timestamp__

    # Config Options
    DEFINES += CONFIG_A2DP_PROCESS_MANAGEMENT
    DEFINES += CONFIG_HSP_PROCESS_MANAGEMENT

    LIBS += -lpthread -lm -lstdc++
    #LIBS += -lsqlite3

    ###########################################################
    #  Set DisplayAudio library include and library path
    NX_DAUDIO_LIB_PATH = $$PWD/../../library

    ###########################################################
    # ipc module
    INCLUDEPATH += $${NX_DAUDIO_LIB_PATH}/include/
    LIBS += -L$${NX_DAUDIO_LIB_PATH}/lib/ -lnxdaudioipc

    ###########################################################
    # daudio utils module
    LIBS += -lnxdaudioutils

    ###########################################################
    # bt mobule
    NX_DAUDIO_BT_LIB_PATH = $${NX_DAUDIO_LIB_PATH}/lib/nxbt1xx
    INCLUDEPATH += $${NX_DAUDIO_BT_LIB_PATH}/include/
    LIBS += -L$${NX_DAUDIO_BT_LIB_PATH}/lib -lnxbt -lappbt -lnrec_hf -lnxalsa

} else {
    error("current toolchain is not supported!")
}
