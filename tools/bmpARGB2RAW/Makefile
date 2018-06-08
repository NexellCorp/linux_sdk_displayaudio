DIR :=
DIR += src

all:
	@for dir in $(DIR); do	\
	make -C $$dir || $$dir || exit $?;	\
	make -C $$dir install;	\
	mkdir -p output; \
	mkdir -p output/out; \
	done

clean:
	@for dir in $(DIR); do	\
	make -C $$dir clean || exit $?;	\
	done

distclean:
	@for dir in $(DIR); do	\
	make -C $$dir distclean || exit $?;	\
	done
	rm -rf output
