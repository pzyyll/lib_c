CXX = g++
RM = rm -f
CXXFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX

LIBS = ../../../libsns.a -lz -lbz2 -lpthread
INCS = -I../../..

TARGETS = hash_db_test hash_db_batch_test hash_db_batch_test2

all:$(TARGETS)

hash_db_test:hash_db_test.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

hash_db_batch_test:hash_db_batch_test.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

hash_db_batch_test2:hash_db_batch_test2.o
	$(CXX) $(INCS) $< $(LIBS) -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe

install:
	jmscp hash_db_test 10.6.206.148 /usr/local/sns10/hash_db_test
	jmscp hash_db_batch_test 10.6.206.148 /usr/local/sns10/hash_db_test
	jmscp hash_db_batch_test2 10.6.206.148 /usr/local/sns10/hash_db_test
	
