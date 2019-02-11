include ../../env.mk

################################################################################
# Build options
INCLUDE += -I.
INCLUDE += -I../../library/include
INCLUDE += -I../../library/prebuilt/include

LIBRARY += -L../../library/lib -lnxdaudioutils
LIBRARY += -L../../library/prebuilt/lib
LIBRARY += -lnxbt -lappbt -lnxalsa -lnxconfig
LIBRARY += -lpthread

################################################################################
# Target
COBJS	:=

CXXOBJS	:=
CXXOBJS += NxBTService.o
CXXOBJS += DAudioIface_Impl.o

CXXFLAGS += -fPIC -DCONFIG_A2DP_PROCESS_MANAGEMENT -DCONFIG_HSP_PROCESS_MANAGEMENT

TARGET := libnxbtservice.so

################################################################################
# Build
OBJS	:= $(COBJS) $(CXXOBJS)

all: $(TARGET)

$(TARGET):	depend $(OBJS)
	$(CXX) -shared -Wl,-soname,$(TARGET) -o $@ $(OBJS) $(LDFLAGS) $(LIBRARY)

install:

clean:
	rm -rf $(TARGET) *.o .depend

distclean: clean

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(AOBJS:.o=.s) $(CXXOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CXX) -M $(CFLAGS) $(AFLAGS) $(CXXFLAGS) $(INCS) $(SRCS) > .depend
