CXX = g++
RM = rm -f
CXXFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX

LIBS = ../../../petlib.a
INCS = -I../../..

TARGETS = webapi_test stcp_tool

all:$(TARGETS)

webapi_test:webapi_test.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

stcp_tool:stcp_tool.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe

