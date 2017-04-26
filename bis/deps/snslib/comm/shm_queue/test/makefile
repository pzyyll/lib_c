CXX = g++
RM = rm -f
CXXFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX

LIBS = ../../../libsns.a
INCS = -I../../..

TARGETS = shm_queue_spped

all:$(TARGETS)

shm_queue_spped: shm_queue_spped.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe

install: all
	jmscp shm_queue_spped 10.12.23.66 /data/ml/tools
