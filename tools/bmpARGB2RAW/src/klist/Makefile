CC := gcc
INC :=
LIBS :=
CFLAGS := $(INC) -fPIC -Wall -Wextra -O2
LDFLAGS := -shared
RM := rm -rf

TARGET_LIB := libklist.so

SRCS := klist.c

OBJS := $(SRCS:.c=.o)

.PHONY: all
all : $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< > $@

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d) .depend

distclean: clean
