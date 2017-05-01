CXX = g++
RM = rm -f
CXXFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX

LIBS = ../../../snslib.a
INCS = -I../../..

TARGETS = file_lock_test

all:$(TARGETS)

file_lock_test:file_lock_test.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

hash_list_nolock_test:hash_list_nolock_test.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe

