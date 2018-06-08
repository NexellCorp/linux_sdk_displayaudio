################################################################################
# 	Build Options
################################################################################
OPTS		:= -Wall -O2 -Wextra -Wcast-align -Wno-unused-parameter \
				-Wshadow -Wwrite-strings -Wcast-qual -fno-strict-aliasing \
				-fstrict-overflow -fsigned-char -fno-omit-frame-pointer \
				-fno-optimize-sibling-calls

COPTS		:= $(OPTS)
CXXOPTS		:= $(OPTS) -Wnon-virtual-dtor

CFLAGS 	 	+= $(OPTS)
CXXFLAGS 	+= $(OPTS)
AFLAGS 		+=

ARFLAGS		+= crv
LDFLAGS  	+=
LIBRARY		:=

ifeq ($(OECORE_SDK_VERSION),2.3.1)
# The SDK 2.3.1 is based on X11.
CFLAGS		+= -DQT_X11
CXXFLAGS	+= -DQT_X11
endif

################################################################################
# 	Generic Rules
################################################################################
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

%.o: %.s
	$(AS) $(AFLAGS) $(INCLUDE) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $@ $<
