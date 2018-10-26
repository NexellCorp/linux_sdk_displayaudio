DIR		:=
######################################################################
# Build options

# Multi Media Libraries
DIR += src

######################################################################
# Build
all:
	@for dir in $(DIR); do			\
	make -C $$dir || exit $?;		\
	make -C $$dir install; 			\
	done

clean:
	@for dir in $(DIR); do			\
	make -C $$dir clean || exit $?;	\
	done

install:
	@for dir in $(DIR); do			\
	make -C $$dir install; 			\
	done

distclean:
	@for dir in $(DIR); do			\
	make -C $$dir distclean || exit $?;	\
	done
