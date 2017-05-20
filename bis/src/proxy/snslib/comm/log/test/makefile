CXX = g++
RM = rm -f
CXXFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX

LIBS = ../../../libsns.a
INCS = -I../../..

TARGETS = log_test

all:$(TARGETS)

log_test:log_test.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe

