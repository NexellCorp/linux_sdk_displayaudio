include ../../env.mk

################################################################################
# Build options
INCLUDE += -I./ -I../../library/external/sqlite-autoconf-3200000/result/include
LIBRARY += -L../../library/external/sqlite-autoconf-3200000/result/lib -lsqlite3

LIBRARY	+= -lstdc++ -lpthread

#LDFLAGS += -Wl, -rpath=/home/doriya/working/solution/display-audio/library/lib

################################################################################
# Target
COBJS	:=
COBJS	+=

CXXOBJS	:=
CXXOBJS += main.o

CFLAGS	+= -g

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
