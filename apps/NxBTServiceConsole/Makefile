include ../../env.mk

#########################################################################
#	Toolchain.
#########################################################################
LIB_DIR	:= ../../library/prebuilt/lib
INC_DIR	:= ../../library/prebuilt/include

INCLUDE	+= -I. -I$(INC_DIR)

LIBRARY	+= -L$(LIB_DIR) -lnxbt -lappbt -lnxalsa
LIBRARY	+= -lstdc++ -lpthread -lm

CXXOBJS	+= main.o

CXXFLAGS += -fPIC

TARGET	= NxBTServiceConsole

# ALSA default audio path
alsa_playback       ?= 'plughw:0,0'
alsa_capture        ?= 'plughw:0,0'
alsa_sco_playback   ?= 'plughw:0,2'
alsa_sco_capture    ?= 'plughw:0,2'

CXXFLAGS += -DNX_ALSA_DEV_NAME_P='"$(alsa_playback)"'
CXXFLAGS += -DNX_ALSA_DEV_NAME_C='"$(alsa_capture)"'
CXXFLAGS += -DNX_ALSA_BT_DEV_NAME_P='"$(alsa_sco_playback)"'
CXXFLAGS += -DNX_ALSA_BT_DEV_NAME_C='"$(alsa_sco_capture)"'

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
