include ../../env.mk

#########################################################################
#	Toolchain.
#########################################################################
INCLUDE	+= -I. -I../../library/prebuilt/include -I../../library/include

LIBRARY += -L../../library/prebuilt/lib -L../../library/lib
LIBRARY	+= -lnxbt -lappbt -lnxalsa -lnx_config
LIBRARY	+= -lstdc++ -lpthread -lm

CXXOBJS	+= main.o

CXXFLAGS += -fPIC

TARGET	= NxBTServiceConsole

OBJS	:= $(COBJS) $(CXXOBJS)

all: $(TARGET)

$(TARGET):	depend $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LIBRARY)

install:
	cp -av $(TARGET) ../../bin/

clean:
	rm -rf $(OBJS) $(TARGET) .depend

distclean: clean
	rm -rf ../../bin/$(TARGET)

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CXXOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(CXXFLAGS) $(INCS) $(SRCS) > .depend
