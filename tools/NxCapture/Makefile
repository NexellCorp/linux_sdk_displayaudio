include ../../env.mk

######################################################################
# Build options
INCLUDE += -I./
LIBRARY += -lpng
LIBRARY	+= -lstdc++ -lpthread

######################################################################
# Target
COBJS	:=
CPPOBJS	:= NxCapture.cpp

CXXFLAGS += -fPIC

TARGET := NxCapture

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET)

$(TARGET):	depend $(OBJS)
	$(CC) $(CXXFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LIBRARY)

install:
	cp -av $(TARGET) ../../bin/

clean:
	rm -f $(TARGET) *.o .depend

distclean: clean
	rm -rf ../../bin/$(TARGET)

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp) $(APPOBJS:.o=.c)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CXXFLAGS) $(INCS) $(SRCS) > .depend
