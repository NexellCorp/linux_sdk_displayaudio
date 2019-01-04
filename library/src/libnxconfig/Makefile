include ../../../env.mk

######################################################################
# Build options
INCLUDE += -I. -I../../include
INCLUDE += -I. -I../../prebuilt/include
ifneq ($(SDKTARGETSYSROOT), )
INCLUDE += -I$(SDKTARGETSYSROOT)/usr/include/libxml2
endif

LIBRARY += -lxml2
LIBRARY	+= -lstdc++ -lpthread -lm

######################################################################
# Target
COBJS	:=
CXXOBJS	:=
CXXOBJS += NX_CConfig.o

CXXFLAGS += -fPIC

TARGET := libnxconfig.so

######################################################################
# Build
OBJS	:= $(COBJS) $(CXXOBJS)

all: $(TARGET)

$(TARGET):	depend $(OBJS)
	$(CC) -shared -Wl,-soname,$(TARGET) -o $@ $(OBJS) $(LDFLAGS) $(LIBRARY)

install:
	cp -av $(TARGET) ../../lib/
	cp -av NX_IConfig.h ../../include/

clean:
	rm -rf $(TARGET) *.o .depend

distclean: clean
	rm -rf ../../lib/$(TARGET)
	rm -rf ../../include/NX_IConfig.h

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(AOBJS:.o=.s) $(CXXOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(AFLAGS) $(CXXFLAGS) $(INCS) $(SRCS) > .depend
