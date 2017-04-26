
#include "shm_queue_unique.h"

#include <iostream>

using namespace std;
using namespace petlib;

typedef unsigned int KeyType;

struct Data{
	KeyType UniqueKey() const { return data; }

	int data;
};

int main(int argc, char *argv[]){
	CShmQueueUnique<KeyType, Data> queue;

	const int CAPACITY = 100000;

	int rv = queue.Init(1985, CAPACITY);
	if(rv != 0){
		cout << "Init failed " << rv << " " << queue.GetErrMsg() << endl;
		return 1;
	}

	for(int i=0; i<CAPACITY; ++i){
		Data d = { i };
		rv = queue.Push(d);
		if(rv != 0)
			cout << "push failed " << rv << endl;
		else
			cout << d.data << " " << queue.Size() << endl;
	}

	while(!queue.Empty()){
		Data d;
		queue.Front(&d);
		cout << d.data;
		queue.Pop(&d);
		cout << " " << d.data << " " << queue.Size() << endl;
	}

	return 0;
}
