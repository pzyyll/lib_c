CXX = g++
RM = rm -f
CXXFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX

LIBS = ../../../libsns.a
INCS = -I../../..

TARGETS = timer_pool_test test_timer

all:$(TARGETS)

timer_pool_test: timer_pool_test.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

test_timer:test_timer.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe

install: all
	jmscp timer_pool_test 10.12.194.36 /data/ml/shimmer
	jmscp test_timer 10.12.194.36 /data/ml/shimmer
