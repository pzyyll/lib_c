/*
 * hdb.h
 *
 *  Created on: 2010-10-17
 *      Author: rayxia
 */

#ifndef __HDB_H__
#define __HDB_H__

#include<stdint.h>
#include<stdio.h>
#include<unistd.h>

namespace snslib{
typedef struct {
	char			magic[16];				// 标识符
	int				fd;						// 文件描述符
	uint64_t		bnum;					// 块总数
	uint64_t		fbnum;					// 空闲块总数
	uint16_t		bsiz;					// 块大小
	uint32_t		hbnum;					// hash桶个数
	uint64_t		fboff;					// 空闲块偏移
	uint64_t		fmaxsiz;				// mmap映射空间大小
	uint64_t		fsiz;					// 文件大小
	char * 			map;					// 映射文件指针
	uint16_t		fbaddnum;				// 每次添加的空闲块个数
}HDB_HEADER;

typedef struct {
	uint64_t 		key;
	uint16_t		size;					// 块大小，最大64k
	uint64_t		next;
	uint64_t		pre;
	uint64_t		down;
	char 			* data[0];
}HDB_BHEADER;

class HDB {
public:
	const static int SUCC		= 0;
	const static int ERROR		= -1;
	const static int NONODE		= -2;

public:
	HDB();
	~HDB();

	// init
	int init( char * fpath, uint16_t bsize, uint32_t hbnum, uint64_t xmsiz );
	// put reocrd
	int put(uint32_t key, char * buf, uint16_t len);
	// get record buf, 'len' must set to 'buf' length
	int get(uint32_t key, char * buf, uint16_t * len );
	// delete record
	int out(uint32_t key);

	char * getemsg(){
		return m_emsg;
	}

private:
	bool lock(int fd);
	bool unlock(int fd);
	bool seekwrite( uint64_t off, const void *buf, size_t size);
	bool seekread(uint64_t off, void *buf, size_t size);
	bool addfb();
	bool retfb(uint64_t off);
	uint32_t hash(uint32_t key);
	bool getfb(uint64_t *poff);

private:
	char 			m_emsg[1024];
	HDB_HEADER		* m_dbheader;
	char 			* m_prbuf;
};
}

#endif /* __HDB_H__ */
