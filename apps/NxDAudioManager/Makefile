include ../../env.mk

################################################################################
# Build options
INCLUDE	+= -I./ -I../../library/include/
INCLUDE	+= -I../../library/lib/sqlite/include/

LIBRARY	+= -L../../library/lib -lnxdaudioipc -lnxdaudioutils
LIBRARY	+= -L../../library/lib/sqlite -lsqlite3

LIBRARY	+= -lstdc++ -lpthread -lcrypto

#LDFLAGS += -Wl, -rpath=/home/doriya/working/solution/display-audio/library/lib

################################################################################
# Target
COBJS	:=
COBJS	+= uevent.o

CXXOBJS	:=
CXXOBJS	+= CNX_Socket.o
CXXOBJS	+= CNX_ProcessManager.o
CXXOBJS	+= CNX_UeventManager.o
CXXOBJS	+= CNX_EventManager.o
CXXOBJS	+= CNX_PostProcess.o
CXXOBJS	+= CNX_File.o
CXXOBJS	+= CNX_MediaDatabase.o
CXXOBJS	+= CNX_MediaScanner.o
CXXOBJS	+= CNX_VolumeManager.o
CXXOBJS	+= CNX_CommandManager.o
CXXOBJS	+= NX_Utils.o
CXXOBJS	+= NX_DbgMsg.o
CXXOBJS	+= NX_DAudioManager.o

#CFLAGS	+= -g

TARGET	:= NxDAudioManager

################################################################################
# Build
OBJS	:= $(COBJS) $(CXXOBJS)

all: $(TARGET)

$(TARGET):	depend $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LIBRARY)

install:
	cp -av $(TARGET) ../../bin/

clean:
	rm -f $(TARGET) *.o .depend

distclean: clean

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(AOBJS:.o=.s) $(CXXOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(AFLGAS) $(CXXFLAGS) $(INCS) $(SRCS) > .depend
