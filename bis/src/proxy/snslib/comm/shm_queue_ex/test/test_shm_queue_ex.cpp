
#include "shm_queue_ex.h"

#include <iostream>

using namespace std;
using namespace petlib;

struct Data{
	int data;
};

struct TestCallback : public CShmQueueEx<Data>::Callback{
	bool Exec(Data &data){
		cout << data.data << endl;
		return true;
	}
};

int main(int argc, char *argv[]){
	CShmQueueEx<Data> queue;

	const int CAPACITY = 100;

	int rv = queue.Init(1985, CAPACITY);
	if(rv != 0){
		cout << "Init failed " << rv << endl;
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

#if 0
	while(!queue.Empty()){
		Data d;
		queue.Front(&d);
		cout << d.data;
		queue.Pop(&d);
		cout << " " << d.data << " " << queue.Size() << endl;
	}

#else 

	TestCallback cb;
	queue.ForEach(&cb);

#endif
	return 0;
}
