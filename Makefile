ifndef	JOBS
JOBJS	:= 8
endif

ifeq ($(OE_QMAKE_AR), )
export OE_QMAKE_AR=$(NX_OE_QMAKE_AR)
endif
ifeq ($(OE_QMAKE_CC), )
export OE_QMAKE_CC=$(NX_OE_QMAKE_CC)
endif
ifeq ($(OE_QMAKE_CFLAGS), )
export OE_QMAKE_CFLAGS=$(NX_OE_QMAKE_CFLAGS)
endif
ifeq ($(OE_QMAKE_CXX), )
export OE_QMAKE_CXX=$(NX_OE_QMAKE_CXX)
endif
ifeq ($(OE_QMAKE_CXXFLAGS), )
export OE_QMAKE_CXXFLAGS=$(NX_OE_QMAKE_CXXFLAGS)
endif
ifeq ($(OE_QMAKE_INCDIR_QT), )
export OE_QMAKE_INCDIR_QT=$(NX_OE_QMAKE_INCDIR_QT)
endif
ifeq ($(OE_QMAKE_LDFLAGS), )
export OE_QMAKE_LDFLAGS=$(NX_OE_QMAKE_LDFLAGS)
endif
ifeq ($(OE_QMAKE_LIBDIR_QT), )
export OE_QMAKE_LIBDIR_QT=$(NX_OE_QMAKE_LIBDIR_QT)
endif
ifeq ($(OE_QMAKE_LINK), )
export OE_QMAKE_LINK=$(NX_OE_QMAKE_LINK)
endif
ifeq ($(OE_QMAKE_QDBUSCPP2XML), )
export OE_QMAKE_QDBUSCPP2XML=$(NX_OE_QMAKE_QDBUSCPP2XML)
endif
ifeq ($(OE_QMAKE_QDBUSXML2CPP), )
export OE_QMAKE_QDBUSXML2CPP=$(NX_OE_QMAKE_QDBUSXML2CPP)
endif
ifeq ($(OE_QMAKE_QT_CONFIG), )
export OE_QMAKE_QT_CONFIG=$(NX_OE_QMAKE_QT_CONFIG)
endif
ifeq ($(OE_QMAKE_MOC), )
export OE_QMAKE_MOC=$(NX_OE_QMAKE_MOC)
endif
ifeq ($(OE_QMAKE_RCC), )
export OE_QMAKE_RCC=$(NX_OE_QMAKE_RCC)
endif
ifeq ($(OE_QMAKE_UIC), )
export OE_QMAKE_UIC=$(NX_OE_QMAKE_UIC)
endif
ifeq ($(OE_QMAKE_PATH_HOST_BINS), )
export OE_QMAKE_PATH_HOST_BINS=$(NX_OE_QMAKE_PATH_HOST_BINS)
endif
ifeq ($(QMAKESPEC), )
export QMAKESPEC=$(NX_OE_XQMAKESPEC)
endif
ifeq ($(QT_CONF_PATH), )
export QT_CONF_PATH=$(NX_QT_CONF_PATH)
endif

QMAKE_PATH=$(OE_QMAKE_PATH_HOST_BINS)

TOP_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

LIB_TOP		:= library/src
APP_TOP		:= apps
TOOL_TOP	:= tools
PODO_TOP	:= $(APP_TOP)/podo
BUILD_TOP	:= qt_build
BIN_TOP		:= bin
RESULT		:= result
RESULT_LIB_TOP := library/lib
QT_INC_TOP	:= library/include

LIBS :=
LIBS += KeyReceiver
LIBS += libnxconfig
LIBS += libnxdaudioutils
LIBS += libavin
LIBS += librearcam

QT_LIBS :=
QT_LIBS += libnxbaseui

APPS :=
APPS += NxBTServiceConsole
APPS += KeyInputSender
APPS += NxCommandSender
APPS += NxBTService

QT_APPS :=
QT_APPS += NxLauncher
QT_APPS += NxBTAudio
QT_APPS += NxBTPhone
QT_APPS += NxBTSettings

QT_APPS += NxAudioPlayer
QT_APPS += NxVideoPlayer

TOOLS :=
TOOLS += NxCapture
TOOLS += NxLogcat

RESULT_LIBS := libnxkeyreceiver.so* libnxconfig.so* libnxbaseui.so* libnxavin.so* libnxrearcam.so* libnxdaudioutils.so*

QT_INC := CNX_BaseDialog.h CNX_KeyboardFrame.h CNX_MediaController.h CNX_MessageBox.h CNX_StatusBar.h

######################################################################
# Build
all:
	mkdir -p $(BIN_TOP)
	@for dir in $(LIBS); do							\
	make -C $(LIB_TOP)/$$dir -j$(JOBS) || exit $?;	\
	make -C $(LIB_TOP)/$$dir install || exit $?;	\
	done
	@for dir in $(APPS); do							\
	make -C $(APP_TOP)/$$dir -j$(JOBS) || exit $?;	\
	make -C $(APP_TOP)/$$dir install || exit $?;	\
	done
	@for dir in $(TOOLS); do						\
	make -C $(TOOL_TOP)/$$dir -j$(JOBS) || exit $?;	\
	make -C $(TOOL_TOP)/$$dir install || exit $?;	\
	done
	@for dir in $(QT_LIBS); do						\
	mkdir -p $(BUILD_TOP)/build-$$dir;				\
	cd $(BUILD_TOP)/build-$$dir; 					\
	$(QMAKE_PATH)/qmake $(TOP_DIR)/$(LIB_TOP)/$$dir || exit $?;	\
	make -j$(JOBS) || exit $?;						\
	make install || exit $?;						\
	cd -; 											\
	done
	@for dir in $(QT_APPS); do						\
	mkdir -p $(BUILD_TOP)/build-$$dir;				\
	cd $(BUILD_TOP)/build-$$dir; 					\
	$(QMAKE_PATH)/qmake $(TOP_DIR)/$(APP_TOP)/$$dir || exit $?;	\
	make -j$(JOBS) || exit $?;						\
	cd -; 											\
	done
	@for dir in $(QT_PODO); do						\
	mkdir -p $(BUILD_TOP)/build-$$dir;				\
	cd $(BUILD_TOP)/build-$$dir; 					\
	$(QMAKE_PATH)/qmake $(TOP_DIR)/$(PODO_TOP)/$$dir || exit $?;\
	make -j$(JOBS) || exit $?;						\
	cd -; 											\
	done
	./tools/scripts/make_package.sh

clean_linux:
	@for dir in $(LIBS); do							\
	make -C $(LIB_TOP)/$$dir clean || exit $?;		\
	done
	@for dir in $(APPS); do							\
	make -C $(APP_TOP)/$$dir clean || exit $?;		\
	done
	@for dir in $(TOOLS); do						\
	make -C $(TOOL_TOP)/$$dir clean || exit $?;		\
	done

distclean_linux:
	@for dir in $(LIBS); do							\
	make -C $(LIB_TOP)/$$dir distclean || exit $?;		\
	done
	@for dir in $(APPS); do							\
	make -C $(APP_TOP)/$$dir distclean || exit $?;		\
	done
	@for dir in $(TOOLS); do						\
	make -C $(TOOL_TOP)/$$dir distclean || exit $?;		\
	done

clean_qt:
	@for dir in $(QT_LIBS); do						\
	if [ -d $(BUILD_TOP)/build-$$dir ] ; then		\
		cd $(BUILD_TOP)/build-$$dir || exit $?;		\
		make clean || exit $?;						\
		cd -; 										\
	fi												\
	done
	@for dir in $(QT_APPS); do						\
	if [ -d $(BUILD_TOP)/build-$$dir ] ; then		\
		cd $(BUILD_TOP)/build-$$dir || exit $?;		\
		make clean || exit $?;						\
		cd -; 										\
	fi												\
	done
	@for dir in $(QT_PODO); do						\
	if [ -d $(BUILD_TOP)/build-$$dir ] ; then		\
		cd $(BUILD_TOP)/build-$$dir || exit $?;		\
		make clean || exit $?;						\
		cd -; 										\
	fi												\
	done

clean_libs:
	@for dir in $(RESULT_LIB_TOP); do				\
	cd $(RESULT_LIB_TOP);							\
	rm -rf $(RESULT_LIBS);							\
	cd -;											\
	done

clean_qt_header:
	@for dir in $(QT_INC_TOP); do					\
	cd $(QT_INC_TOP);								\
	rm -rf $(QT_INC);								\
	cd -;											\
	done

clean:
	make clean_linux
	@if [ -d ${BUILD_TOP} ] ; then					\
		make clean_qt;								\
	fi

distclean:
	make distclean_linux
	make clean_libs
	make clean_qt_header
	rm -rf $(BIN_TOP)
	rm -rf $(BUILD_TOP)
	rm -rf $(RESULT)

package:
	./tools/scripts/make_package.sh

push:
	./tools/scripts/push.sh
