########################################################################
# Get Linux Build Enviornment:
include ../env.mk

CFLAGS		+= -fPIC
CPPFLAGS	+= -fPIC

#########################################################################
#																		#
#								Includes								#
#																		#
#########################################################################
#	Local Application Includes
INCLUDE		+= -I./ -I../../../include
ifneq ($(SDKTARGETSYSROOT), )
INCLUDE		+= -I$(SDKTARGETSYSROOT)/usr/include/drm
endif

#########################################################################
#																		#
#								Sources									#
#																		#
#########################################################################
COBJS		:=
CPPOBJS		:=	main.o

#########################################################################
#																		#
#								Library									#
#																		#
#########################################################################
LIBRARY 	+= -L../../../lib -lnxavin -lnx_video_api -lnx_v4l2 -ldrm
LIBRARY		+= -lpthread -lstdc++

#########################################################################
#																		#
#								Target									#
#																		#
#########################################################################
# Target
TARGET		:= test_avin

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LIBRARY)
	@echo "done"

install:

clean:
	rm -f $(TARGET) *.o .depend

distclean: clean

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC)  -M  $(CXXFLAGS)   $(INCS) $(SRCS) > .depend
