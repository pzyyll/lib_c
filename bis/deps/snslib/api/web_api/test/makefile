CPP = g++
RM = rm -f
CPPFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX

BASE_DIR = ../../../..

INCS = -I.\
       -I$(BASE_DIR)/ \
       -I${BASE_DIR}/snslib/ 
       
LIBS = $(BASE_DIR)/snslib/libsns.a -lz -lbz2 -lpthread -ldl $(BASE_DIR)/ext/tsf4g_base_2_1_13853_64/lib/libtsf4g_r.a  $(BASE_DIR)/ext/tsf4g_base_2_1_13853_64/lib/libscew.a  $(BASE_DIR)/ext/tsf4g_base_2_1_13853_64/lib/libexpat.a /usr/local/lib64/libprotobuf.a \
		/usr/local/lib/libtokyotyrant.a /usr/local/lib/libtokyocabinet.a




TARGETS = test_replay

all:$(TARGETS)

test_replay: TestReplay.o ../webapi.o $(BASE_DIR)/snslib/api/proto/flash_new.pb.o
	$(CPP) $(INCS) $^ $(LIBS) -o $@

TestReplay.o: TestReplay.cpp
	$(CPP) -c $(CPPFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe