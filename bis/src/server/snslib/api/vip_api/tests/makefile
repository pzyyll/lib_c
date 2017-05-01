CPP = g++
RM = rm -f

CPPFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX
INCS = -I. -I.. -I../../../ -I../../../../ext/c4b_4.1/include
LIBS = ../../../petlib.a \
	   ../../../../ext/c4b_4.1/lib/libc4b_api.a \
	   ../../../../ext/c4b_4.1/lib/libdnscli_V1.1R130.a

TARGETS = c4b

all:$(TARGETS)

c4b: c4b.o
	$(CPP) $(INCS) $^ $(LIBS) -o $@

%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe

