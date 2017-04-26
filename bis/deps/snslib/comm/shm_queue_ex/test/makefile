PETLIB=../../..

all: test_shm_queue_ex test_shm_queue_unique

SRC = ../shm_queue_ex.cpp ../shm_queue_unique.cpp $(PETLIB)/comm/sem_lock/sem_lock.cpp
DEP = $(SRC) ../shm_queue_ex.h ../shm_queue_unique.h

test_shm_queue_ex: test_shm_queue_ex.cpp $(DEP)
	g++ -Wall -g -DLINUX -I$(PETLIB) -I.. $(SRC) $< -o $@

test_shm_queue_unique: test_shm_queue_unique.cpp $(DEP)
	g++ -Wall -g -DLINUX -I$(PETLIB) -I.. $(SRC) $< -o $@

clean:
	rm test_shm_queue_ex test_shm_queue_unique
