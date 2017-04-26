CXX = g++
RM = rm -f
CXXFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX

LIBS = 
INCS = -I../../..

TARGETS = fmp_test fmp_bench_test pet_map_test

all:$(TARGETS)

pet_map_test:pet_map_test.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

fmp_test:fmp_test.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

fmp_bench_test:fmp_bench_test.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe

