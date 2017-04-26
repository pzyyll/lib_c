CXX = g++
RM = rm -f
CXXFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX

LIBS = ../../../libsns.a /usr/local/lib/libtokyotyrant.a /usr/local/lib/libtokyocabinet.a -lz -lbz2 -lpthread
INCS = -I../../..

TARGETS = tt_feeds_test# tt_feeds_batch_test

all:$(TARGETS)

tt_feeds_test:tt_feeds_test.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

#tt_feeds_batch_test:tt_feeds_batch_test.o
#	$(CXX) $(INCS) $< $(LIBS) -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe

