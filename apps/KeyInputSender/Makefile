include ../../env.mk

######################################################################
# Build options
INCLUDE	+= -I./
LIBRARY	+= -lstdc++ -lpthread

######################################################################
# Target
COBJS	:=
CPPOBJS	:= main.o

TARGET	:= key_input

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

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

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp) $(APPOBJS:.o=.c)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
