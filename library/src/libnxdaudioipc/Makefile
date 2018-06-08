include ../../../env.mk

######################################################################
# Build options
INCLUDE += -I./ -I../../include
LIBRARY	+= -lstdc++ -lpthread -lm

######################################################################
# Target
COBJS	:=
CXXOBJS	:=
CXXOBJS += NX_IpcPacket.o
CXXOBJS += CNX_IpcManager.o
CXXOBJS += NX_IpcUtils.o
CXXOBJS += CNX_Semaphore.o

CXXFLAGS += -fPIC

TARGET := libnxdaudioipc.so

######################################################################
# Build
OBJS	:= $(COBJS) $(CXXOBJS)

all: $(TARGET)

$(TARGET):	depend $(OBJS)
	$(CC) -shared -Wl,-soname,$(TARGET) -o $@ $(OBJS) $(LDFLAGS) $(LIBRARY)

install:
	cp -av $(TARGET) ../../lib/
	cp -av INX_IpcManager.h ../../include/
	cp -av NX_IpcPacket.h ../../include/
	cp -av NX_IpcUtils.h ../../include/

clean:
	rm -f $(TARGET) *.o .depend

distclean: clean
	rm -f ../../lib/$(TARGET)
	rm -f ../../include/INX_IpcManager.h
	rm -f ../../include/NX_IpcPacket.h
	rm -f ../../include/NX_IpcUtils.h

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(AOBJS:.o=.s) $(CXXOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(AFLGAS) $(CXXFLAGS) $(INCS) $(SRCS) > .depend
