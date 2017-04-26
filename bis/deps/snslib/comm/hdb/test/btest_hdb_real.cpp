#include "hdb.h"
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

using namespace snslib;
int main(int argc, char *argv[]){
	if(argc < 5){
		printf("using <bin> <bsiz> <hnum> <param> <sleep>... \n" );
		return -1;
	}

	HDB hdb;
	uint16_t bsiz = (uint16_t)atoi(argv[1]);
	uint32_t hnum = (uint32_t)atoi(argv[2]);
	int param = atoi(argv[3]);
	int ssec = atoi(argv[4]);
	int rt = hdb.init( "./btest.hdb", bsiz, hnum, 200*1024*1024*1024ll );
	if(rt){
		printf("%s\n",hdb.getemsg());
		return -1;
	}

	char szExp[10*1024];
	memset(szExp, 0, sizeof(szExp));
	for(int i=0; i<sizeof(szExp)/10; i++){
		snprintf( szExp + i*10, sizeof(szExp), "%s", "1234567890" );
	}

	struct timeval stbegin, stend, stspeeds, stspeede;
	gettimeofday(&stbegin, NULL);
	stspeeds = stbegin;
	int r_num = 0, w_num = 0, o_num = 0, r_num_ok = 0, w_num_ok = 0, o_num_ok = 0;
	int pnum = 10000;
	srand(1000);
	while(true){
	int cmd = 0;
	if(param <= 3){
		cmd = param;
	}else{
		cmd = rand()%3 + 1;
	}
	unsigned int key = 10000000 + rand()%10000000 ;
//	printf("gen key=%d and cmd=%d\n", key, cmd );
	switch(cmd){
		case 1:{
			rt = hdb.put(key, szExp, sizeof(szExp));
			if(rt){
				printf("%s\n",hdb.getemsg());
				return -1;
			}
			r_num++;
			r_num_ok++;
			break;
		}

		case 2:{
			char store[64000];
			unsigned short len=sizeof(store);

			rt = hdb.get(key, store, &len);
			if( rt == HDB::SUCC  ){
				if( memcmp( szExp, store, sizeof(szExp) ) ){
					printf("get cmp failed");
					return -1;
				}
				w_num_ok++;
			}else if(rt == HDB::NONODE ){
			}else{
				printf("%s\n",hdb.getemsg());
				return -1;
			}
			w_num++;
			break;
		}

		case 3:{
			rt = hdb.out(key);
			if( rt == HDB::SUCC ){
				o_num_ok++;
			}else if(rt == HDB::NONODE ){
			}else{
				printf("%s\n",hdb.getemsg());
				return -1;
			}
			o_num++;
			break;
		}

		default:
			break;
	}

	if(r_num + w_num + o_num > 100000){
		gettimeofday(&stspeede, NULL);
		int sgap = (stspeede.tv_sec - stspeeds.tv_sec)*1000000 + (stspeede.tv_usec - stspeeds.tv_usec);
		printf("TOTAL TIME:%dus, speed=%f\n", sgap, ( r_num + w_num + o_num )/ ( sgap / 1000000.0 ));
		printf("W speed:%f, num=%d,%d, ratio=%f\n", r_num / ( sgap / 1000000.0 ), r_num_ok, r_num, r_num_ok * 1.0 / r_num );
		printf("R speed:%f, num=%d,%d, ratio=%f\n", w_num / ( sgap / 1000000.0 ), w_num_ok, w_num, w_num_ok * 1.0 / w_num  );
		printf("O speed:%f, num=%d, %d, ratio=%f\n\n", o_num / ( sgap / 1000000.0 ), o_num_ok, o_num, o_num_ok * 1.0 / o_num  );
		r_num = 0;
		r_num_ok = 0;
		w_num = 0;
		w_num_ok = 0;
		o_num = 0;
		o_num_ok = 0;
		usleep(ssec*1000000);
		gettimeofday(&stspeede, NULL);
		stspeeds = stspeede;
	}
	}

	return 0;
}
