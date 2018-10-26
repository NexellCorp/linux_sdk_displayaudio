include ../../env.mk

#########################################################################
#	Toolchain.
#########################################################################
LIB_DIR	:= ../../library/prebuilt/lib
INC_DIR	:= ../../library/prebuilt/include
BIN_DIR	:= ../../bin

CFLAGS	+= -g
INCLUDE	+= -I./ -I$(INC_DIR)

LIBRARY	+= -L$(LIB_DIR) -lnxbt -lappbt -lnxalsa -lnrec_hf
LIBRARY	+= -lstdc++ -lpthread -lm

CXXOBJS	+= main.o

TARGET	= NxBTServiceConsole

OBJS	:= $(COBJS) $(CXXOBJS)

all: $(TARGET)

$(TARGET):	depend $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LIBRARY)

install:
	cp $(TARGET) $(BIN_DIR)

clean:
	rm -rf $(OBJS) $(TARGET) core .depend

distclean: clean

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CXXOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(CXXFLAGS) $(INCS) $(SRCS) > .depend
