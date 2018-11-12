########################################################################
# Get Linux Build Enviornment:
include ../../../../env.mk

CFLAGS		+= -fPIC
CPPFLAGS	+= -fPIC

#########################################################################
#																		#
#								Includes								#
#																		#
#########################################################################
#	Local Application Includes
INCLUDE		+= -I./ -I../../../include
INCLUDE		+= -I./ -I../../../prebuilt/include
ifneq ($(SDKTARGETSYSROOT), )
INCLUDE		+= -I$(SDKTARGETSYSROOT)/usr/include/drm
endif

#########################################################################
#																		#
#								Sources									#
#																		#
#########################################################################
COBJS		:=
# Library Source
CPPOBJS		:=	NX_CV4l2Camera.o \
				NX_CDrmDisplay.o \
				NX_CBackgearDetect.o
# Utils
CPPOBJS		+=	NX_DbgMsg.o \
				NX_CGpioControl.o
# Manager
CPPOBJS		+=	NX_CRearCamMgr.o

#########################################################################
#																		#
#								Library									#
#																		#
#########################################################################
LIBRARY 	+= -lnx_video_api -lnx_v4l2  -ldrm

#########################################################################
#																		#
#								Target									#
#																		#
#########################################################################
# Target
TARGET		:= libnxrearcam.so

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(TARGET) -o $@ $(OBJS) $(LIBRARY)
	@echo "done"

install:
	cp -a $(TARGET) ../../../lib/
	cp -a NX_RearCam.h ../../../include/

clean:
	rm -f $(TARGET) *.o .depend

distclean: clean
	rm -f ../../../include/NX_RearCam.h

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CXXFLAGS)   $(INCS) $(SRCS) > .depend
