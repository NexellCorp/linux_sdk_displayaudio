include ../../env.mk

################################################################################
# Build options
INCLUDE	+= -I./ -I../../library/include -I../NxDAudioManager
LIBRARY	+= -L./ -L../../library/lib

LIBRARY	+= -lstdc++ -lpthread

################################################################################
# Target
COBJS	:=
COBJS	+=

CXXOBJS	:=
CXXOBJS	+= CNX_Socket.o
CXXOBJS	+= main.o

TARGET	:= NxCommandSender

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
