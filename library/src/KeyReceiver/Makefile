include ../../../env.mk

######################################################################
# Build options
INCLUDE += -I./
LIBRARY	+= -lstdc++ -lpthread -lm

######################################################################
# Target
COBJS	:=
CPPOBJS += CKeyProcessing.o
CPPOBJS += RemoteKeyServer.o
CPPOBJS += DAudioKeyDef.o

CXXFLAGS += -fPIC

TARGET := libnxkeyreceiver.so

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET):	depend $(OBJS)
	$(CC) -shared -Wl,-soname,$(TARGET) -o $@ $(OBJS) $(LDFLAGS) $(LIBRARY)

install:
	cp -av $(TARGET) ../../lib/
	cp -av NX_KeyReceiver.h ../../include/
	cp -av DAudioKeyDef.h ../../include/

clean:
	rm -f $(TARGET) *.o .depend

distclean: clean
	rm -f ../../include/NX_KeyReceiver.h
	rm -f ../../include/DAudioKeyDef.h

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp) $(APPOBJS:.o=.c)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
