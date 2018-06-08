include ../../env.mk

######################################################################
# Build options
INCLUDE += -I./ -I../../library/include
LIBRARY += -L./ -L../../library/lib -lnxconfig -lxml2

LIBRARY	+= -lstdc++ -lpthread

######################################################################
# Target
COBJS	:=
CXXOBJS	:=
CXXOBJS += main.o

TARGET := NxTestConfig

CXXFLAGS += -g

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
