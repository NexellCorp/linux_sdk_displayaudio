include ../../env.mk

################################################################################
# Build options
LIBRARY += -lsqlite3
LIBRARY	+= -lstdc++ -lpthread

################################################################################
# Target
COBJS	:=
COBJS	+=

CXXOBJS	:=
CXXOBJS += main.o

TARGET := NxTestSqlite

################################################################################
# Build
OBJS	:= $(COBJS) $(CXXOBJS)

all: $(TARGET)

$(TARGET):	depend $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LIBRARY)

install:
	cp -av $(TARGET) ../../bin/

clean:
	rm -rf $(TARGET) *.o .depend

distclean: clean
	rm -rf ../../bin/$(TARGET)

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(AOBJS:.o=.s) $(CXXOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(AFLAGS) $(CXXFLAGS) $(INCS) $(SRCS) > .depend
