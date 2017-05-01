CPP = g++
RM = rm -f
CPPFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX -O2

INCS = -I. -I../../../../snslib -I../../../../ext/boost_1_43_0

LIBS = ../../../../snslib/libsns.a -lz -lbz2 -lpthread -ldl -lrt

TARGETS = buffer_test 

all:$(TARGETS)

buffer_test: test_buffers.o
	$(CPP) $(INCS) $^ $(LIBS) -o $@

%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe

