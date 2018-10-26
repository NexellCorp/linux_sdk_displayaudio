include ../../../env.mk

######################################################################
# Build options
INCLUDE += -I./ -I../../include
INCLUDE += -I./ -I../../prebuilt/include
ifneq ($(SDKTARGETSYSROOT), )
INCLUDE += -I$(SDKTARGETSYSROOT)/usr/include/libxml2
endif

LIBRARY += -L./ -lxml2
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
	rm -f $(TARGET) *.o .depend

distclean: clean
	rm -f ../../lib/$(TARGET)
	rm -f ../../include/NX_IConfig.h

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(AOBJS:.o=.s) $(CXXOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(AFLGAS) $(CXXFLAGS) $(INCS) $(SRCS) > .depend
