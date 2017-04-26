CXX = g++
RM = rm -f
CXXFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX

LIBS = ../../../snslib.a
INCS = -I../../..

TARGETS = hash_list_nolock_test hash_list_mmap_test

all:$(TARGETS)

hash_list_mmap_test:hash_list_mmap_test.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

hash_list_nolock_test:hash_list_nolock_test.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe

