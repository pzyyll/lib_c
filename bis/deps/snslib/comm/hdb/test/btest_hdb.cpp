#include "hdb.h"
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

using namespace snslib;
int main(int argc, char *argv[]){
	if(argc < 6){
		printf("using <bin> <bsiz> <hnum> <cmd> <loop> <key>  ... \n" );
		return -1;
	}

	HDB hdb;
	uint16_t bsiz = (uint16_t)atoi(argv[1]);
	uint32_t hnum = (uint32_t)atoi(argv[2]);
	int rt = hdb.init( "./btest.hdb", bsiz, hnum, 80*1024*1024*1024ll );
	if(rt){
		printf("%s\n",hdb.getemsg());
		return -1;
	}

	int cmd = atoi(argv[3]);
	int loop = atoi(argv[4]);
	unsigned int key = (unsigned int)atoll(argv[5]);
	struct timeval stbegin, stend, stspeeds, stspeede;
	gettimeofday(&stbegin, NULL);
	stspeeds = stbegin;
	int total = loop;
	int num = 0;
	int pnum = 10000;
	switch(cmd){
		case 1:{
			if(argc<7) {
				printf("ERROR: input key's buf \n" );
				return -1;
			}
			while(loop>0){
				rt = hdb.put(key+loop, argv[6], strlen(argv[6])+1);
				if(rt){
					printf("%s\n",hdb.getemsg());
					return -1;
				}
				printf("put ok, key=%lu, msg=%s\n", key+loop, hdb.getemsg());
				loop--;
				num++;
				if(num>pnum){
					num = 0;
					gettimeofday(&stspeede, NULL);
					int sgap = (stspeede.tv_sec - stspeeds.tv_sec)*1000000 + (stspeede.tv_usec - stspeeds.tv_usec);
					printf("write speed:%f\n", pnum / ( sgap / 1000000.0 ) );
					stspeeds = stspeede;
				}
			}
			break;
		}
		case 2:{
			char store[1024];
			unsigned short len=sizeof(store);
			while(loop>0){
				rt = hdb.get(key+loop, store, &len);
				if(rt){
					printf("%s\n",hdb.getemsg());
					return -1;
				}
				printf("read end. key=%lu, str=%s. len=%d\n", key+loop, store, len);
				loop--;
				num++;
				if(num>pnum){
					num = 0;
					gettimeofday(&stspeede, NULL);
					int sgap = (stspeede.tv_sec - stspeeds.tv_sec)*1000000 + (stspeede.tv_usec - stspeeds.tv_usec);
					printf("read speed:%f\n", pnum / ( sgap / 1000000.0 ) );
					stspeeds = stspeede;
				}
			}
			break;
		}
		case 3:{
			while(loop>0){
				rt = hdb.out(key+loop);
				if(rt){
					printf("%s\n",hdb.getemsg());
					return -1;
				}
				printf("out ok. key=%lu, msg=%s\n", key+loop, hdb.getemsg());
				loop--;
				num++;
				if(num>pnum){
					num = 0;
					gettimeofday(&stspeede, NULL);
					int sgap = (stspeede.tv_sec - stspeeds.tv_sec)*1000000 + (stspeede.tv_usec - stspeeds.tv_usec);
					printf("del speed:%f\n", pnum / ( sgap / 1000000.0 ) );
					stspeeds = stspeede;
				}
			}
			break;
		}
		default:
			break;
	}

	gettimeofday(&stend, NULL);
	int gap = (stend.tv_sec - stbegin.tv_sec)*1000000 + (stend.tv_usec - stbegin.tv_usec);
	printf("total speed:%f\n", total / ( gap / 1000000.0 ) );

	return 0;
}
