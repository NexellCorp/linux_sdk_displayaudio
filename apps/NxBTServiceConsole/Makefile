include ../../env.mk

#########################################################################
#	Toolchain.
#########################################################################
LIB_DIR	:= ../../library/prebuilt/lib
LIB_DIR	+= -L../../library/lib
INC_DIR	:= ../../library/prebuilt/include
INC_DIR	+= -I../../library/include

INCLUDE	+= -I. -I$(INC_DIR)

LIBRARY	+= -L$(LIB_DIR) -lnxbt -lappbt -lnxalsa -lnxconfig
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
