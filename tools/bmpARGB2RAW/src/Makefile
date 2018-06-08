CC := gcc
INCLUDE += -I./
INCLUDE += -I./klist

OPTS := -Wall -O2 -Wextra -Wcast-align -Wno-unused-parameter -Wshadow -Wwrite-strings -Wcast-qual -Wno-unused-result -fno-strict-aliasing -fstrict-overflow -fsigned-char -fno-omit-frame-pointer -fno-optimize-sibling-calls

CFLAGS := -g $(INCLUDE) $(OPTS)

COBJS := bmp2raw.o ./klist/klist.o

APP := bmp2raw

all: $(APP)

$(APP):	depend $(COBJS)
	$(CC) $(CFLAGS) $(COBJS) -o $@ $(LIBRARY)
	cp -a $(APP) ../bin

install: $(APP)
	cp $^ ../bin

clean:
	rm -f *.o klist/*.o klist/.depend *.a *.so *.so.* .depend $(APP)

distclean: clean
	rm -rf ../bin/$(APP)

#############################################################################
# Dependency
ifeq (.depend,$(whildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
