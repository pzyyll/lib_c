AUTOMAKE_OPTIONS=foreign

SUBDIRS= comm api

noinst_LIBRARIES=libsns.a

LIB_FILES=$(shell find $(SUBDIRS) -name "*.a")
libsns.a: $(LIB_FILES)
	for i in $?; do ar x $$i; done
	ar cur $@ *.o
	ranlib $@
	rm -f *.o
