#include "hdb.h"
#include <string.h>
#include <stdlib.h>


using namespace snslib;

int main(int argc, char *argv[]){
	if(argc < 3){
		printf("using <bin> <cmd> <key> ... \n" );
		return -1;
	}

	HDB hdb;
	int rt = hdb.init( "./test.hdb", 1024, 10000000, 4*1024*1024*1024ll );
	if(rt){
		printf("%s\n",hdb.getemsg());
		return -1;
	}

	int cmd = atoi(argv[1]);
	unsigned int key = (unsigned int)atoll(argv[2]);
	switch(cmd){
		case 1:{
			if(argc<4) {
				printf("ERROR: input key's buf \n" );
				return -1;
			}
			rt = hdb.put(key, argv[3], strlen(argv[3])+1);
			if(rt){
				printf("%s\n",hdb.getemsg());
				return -1;
			}
			break;
		}
		case 2:{
			char store[1024];
			unsigned short len=sizeof(store);
			rt = hdb.get(key, store, &len);
			if(rt){
				printf("%s\n",hdb.getemsg());
				return -1;
			}
			printf("read end. str=%s. len=%d\n", store, len);
			break;
		}
		case 3:{
			rt = hdb.out(key);
			if(rt){
				printf("%s\n",hdb.getemsg());
				return -1;
			}
			break;
		}
		default:
			break;
	}

	return 0;
}
