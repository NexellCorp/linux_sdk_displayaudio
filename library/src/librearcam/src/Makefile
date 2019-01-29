########################################################################
# Get Linux Build Enviornment:
include ../../../../env.mk

CFLAGS		+= -fPIC
CPPFLAGS	+= -fPIC
CXXFLAGS	+= -fPIC

#########################################################################
#																		#
#								Includes								#
#																		#
#########################################################################
#	Local Application Includes
INCLUDE		+= -I. -I../../../include
INCLUDE		+= -I. -I../../../prebuilt/include
ifneq ($(SDKTARGETSYSROOT), )
INCLUDE		+= -I$(SDKTARGETSYSROOT)/usr/include/drm \
				-I/$(SDKTARGETSYSROOT)/usr/include/nexell
endif

#########################################################################
#																		#
#								Sources									#
#																		#
#########################################################################

CPPOBJS		:=	NX_CV4l2Camera.o \
				NX_CV4l2VipFilter.o \
				NX_CDeinterlaceFilter.o \
				NX_CVideoRenderFilter.o \
				nxp_video_alloc_ion.o \
#				parking_line.o \

COBJS		:= nxp_video_alloc_drm.o \

#COBJS += $(wildcard ./../libtsdeinterlacer.a)

# BackGear Detection
CPPOBJS		+=	NX_CGpioControl.o \
				NX_CBackgearDetect.o \
# Utils
CPPOBJS		+=	NX_DbgMsg.o \

# Manager
CPPOBJS		+=	NX_CRearCamManager.o \

#########################################################################
#																		#
#								Library									#
#																		#
#########################################################################
LDFLAGS += \
		-L../lib \

LIBRARY 	+= -lnx_v4l2  -ldrm  -lnxdeinterlace
#LIBRARY 	+= -lnxv4l2  -ldrm  -lnxdeinterlace -ltsdeinterlacer

#########################################################################
#																		#
#								Target									#
#																		#
#########################################################################
# Target
TARGET		:= libnxrearcam.so

######################################################################
# Build

#CXXFLAGS 	+= -DTHUNDER_DEINTERLACE -DUI_OVERLAY_APP
CXXFLAGS 	+= -DUI_OVERLAY_APP

OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
		$(CC) $(LDFLAGS) -shared -Wl,-soname,$(TARGET) -o $@ $(OBJS) $(LIBRARY)

install:
	cp -av $(TARGET) ../../../lib/
	cp -av NX_RearCam.h ../../../include/

clean:
	rm -rf $(TARGET) *.o .depend

distclean: clean
	rm -rf ../../../lib/$(TARGET)
	rm -rf ../../../include/NX_QuickRearCam.h

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CXXFLAGS)   $(INCS) $(SRCS) > .depend