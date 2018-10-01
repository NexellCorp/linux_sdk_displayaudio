include ../../env.mk

################################################################################
# Build options
INCLUDE += -I./
INCLUDE += -I../../library/include/
INCLUDE += -I../../library/lib/nxbt1xx/include

LIBRARY += -L../../library/lib/ -lnxdaudioutils
LIBRARY += -L../../library/lib/nxbt1xx/lib
LIBRARY += -lnxbt -lappbt -lnrec_hf -lnxalsa
LIBRARY += -lpthread

#LDFLAGS += -Wl, -rpath=/home/doriya/working/solution/display-audio/library/lib

################################################################################
# Target
COBJS	:=

CXXOBJS	:=
CXXOBJS += NxBTService.o
CXXOBJS += DAudioIface_Impl.o

CFLAGS	+= -g
CXXFLAGS += -g -fPIC -std=c++11 -DCONFIG_A2DP_PROCESS_MANAGEMENT -DCONFIG_HSP_PROCESS_MANAGEMENT

TARGET := libnxbtservice.so

################################################################################
# Build
OBJS	:= $(COBJS) $(CXXOBJS)

all: $(TARGET)

$(TARGET):	depend $(OBJS)
	$(CXX) -shared -Wl,-soname,$(TARGET) -o $@ $(OBJS) $(LDFLAGS) $(LIBRARY)

push:
	adb push $(TARGET) /nexell/daudio/NxBTService/

install:

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
	$(CXX) -M $(CFLAGS) $(AFLGAS) $(CXXFLAGS) $(INCS) $(SRCS) > .depend
