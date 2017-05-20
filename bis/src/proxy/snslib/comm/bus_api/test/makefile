CXX = g++
RM = rm -f
CXXFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX

TSF4G_BASE_DIR = ../../../../ext/tsf4g_base_2_1_13853_64

LIBS = ../../../libsns.a \
	   $(TSF4G_BASE_DIR)/lib/libtsf4g.a \
	   $(TSF4G_BASE_DIR)/lib/libscew.a \
	   $(TSF4G_BASE_DIR)/lib/libexpat.a \
	   -lpthread
	
INCS = -I../../..

TARGETS = bus_recv bus_send send_to_router bus_recv2

all:$(TARGETS)

send_to_router:send_to_router.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

bus_recv:bus_recv.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

bus_recv2:bus_recv2.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

bus_send:bus_send.o
	$(CXX) $(INCS) $< $(LIBS) -o $@
	
%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe
	
install:
	jmscp bus_send 10.12.194.36 /data/ml/tbusd/test
	jmscp bus_recv 10.12.194.36 /data/ml/tbusd/test
	jmscp send_to_router 10.12.194.36 /data/ml/tbusd/test
	jmscp bus_recv2 10.12.194.36 /data/ml/tbusd/test
