ifndef	JOBS
JOBJS	:= 8
endif

TOP_DIR		:= $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
SDK_TOP		:= $(TOP_DIR)/../..
BUILD_TOP	:= build

QT_APPS		:=
QT_APPS		+= core
QT_APPS		+= pdwindowcompositor

######################################################################
#
#	Build
#
all:
	@for dir in $(QT_APPS); do							\
	mkdir -p $(SDK_TOP)/$(BUILD_TOP)/build-$$dir;		\
	cd $(SDK_TOP)/$(BUILD_TOP)/build-$$dir;				\
	qmake $(TOP_DIR)/$$dir || exit $?;					\
	make -j$(JOBS) || exit $?;							\
	cd -;												\
	done

clean:
	@for dir in $(QT_APPS); do							\
	cd $(SDK_TOP)/$(BUILD_TOP)/build-$$dir || exit $?;	\
	make clean || exit $?;								\
	cd -;												\
	done

