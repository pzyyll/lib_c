CPP = g++
RM = rm -f

CPPFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX
INCS = -I. -I.. -I../../../
#LIBS = ../../../petlib.a
LIBS = ../libpetvip.a

TARGETS = vip_proxy_tool

all:$(TARGETS)

vip_proxy_tool: vip_proxy_tool.o
	$(CPP) $(INCS) $^ $(LIBS) -o $@

%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe

