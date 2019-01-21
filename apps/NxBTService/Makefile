include ../../env.mk

################################################################################
# Build options
INCLUDE += -I.
INCLUDE += -I../../library/include
INCLUDE += -I../../library/prebuilt/include

LIBRARY += -L../../library/lib -lnxdaudioutils
LIBRARY += -L../../library/prebuilt/lib
LIBRARY += -lnxbt -lappbt -lnxalsa
LIBRARY += -lpthread

################################################################################
# Target
COBJS	:=

CXXOBJS	:=
CXXOBJS += NxBTService.o
CXXOBJS += DAudioIface_Impl.o

CXXFLAGS += -fPIC -DCONFIG_A2DP_PROCESS_MANAGEMENT -DCONFIG_HSP_PROCESS_MANAGEMENT

TARGET := libnxbtservice.so

# ALSA default audio path
alsa_playback       ?= "plughw:0,0"
alsa_capture        ?= "plughw:0,0"
alsa_sco_playback   ?= "plughw:0,2"
alsa_sco_capture    ?= "plughw:0,2"

CXXFLAGS += -DNX_ALSA_DEV_NAME_P='"$(alsa_playback)"'
CXXFLAGS += -DNX_ALSA_DEV_NAME_C='"$(alsa_capture)"'
CXXFLAGS += -DNX_ALSA_BT_DEV_NAME_P='"$(alsa_sco_playback)"'
CXXFLAGS += -DNX_ALSA_BT_DEV_NAME_C='"$(alsa_sco_capture)"'

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
