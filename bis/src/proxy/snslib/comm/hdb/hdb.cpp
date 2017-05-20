/*
 * hdb.cpp
 *
 *  Created on: 2010-10-17
 *      Author: rayxia
 */

#include "hdb.h"
#include<vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/mman.h>

using namespace std;
using namespace snslib;


#define HDBMAGICDATA   		"Tencent Ray.Xia"   		// magic data for identification
#define HDBHEADSIZ     		1024               			// size of the reagion of the header
#define HDBDEFXMSIZE		16*1024*1024*1024			// default xmsize, 16G
#define HDBMAXBSIZE			64*1024						// max block size
#define HDBFBADDNUM			1024						// 每次增加的空闲块

HDB::HDB(){
	m_prbuf = NULL;
	memset( m_emsg, 0, sizeof(m_emsg) );
}

HDB::~HDB(){
	if(m_prbuf){
		delete []m_prbuf;
		m_prbuf = NULL;
	}
}

int HDB::init( char * fpath, uint16_t bsize, uint32_t hbnum, uint64_t fmaxsiz ){
	assert(fpath && bsize > 0 && hbnum > 0);

	if(fmaxsiz < HDBHEADSIZ + sizeof(uint64_t)*hbnum + HDBFBADDNUM * (sizeof(HDB_BHEADER) + bsize)){
		snprintf( m_emsg, sizeof(m_emsg), "fmaxsiz size is too small, file size must > %u, errmsg:[%s]",
				HDBHEADSIZ + sizeof(uint64_t)*hbnum + HDBFBADDNUM * (sizeof(HDB_BHEADER) + bsize), strerror(errno) );
		return ERROR;
	}

	int fd = open( fpath, O_RDWR | O_CREAT, 00644 );
	if(fd < 0){
		snprintf( m_emsg, sizeof(m_emsg), "open file:[%s] failed, errmsg:[%s]", fpath, strerror(errno) );
		return ERROR;
	}

	struct stat sbuf;
	if(fstat(fd, &sbuf) == -1 || !S_ISREG(sbuf.st_mode)){
		snprintf( m_emsg, sizeof(m_emsg), "fstat error, errmsg:[%s]", strerror(errno) );
		close(fd);
		return ERROR;
	}

	char * map = (char *)mmap(0, fmaxsiz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		snprintf( m_emsg, sizeof(m_emsg), "mmap error, errmsg:[%s]", strerror(errno) );
		close(fd);
		return ERROR;
	}

	m_dbheader = (HDB_HEADER *)map;

	if (sbuf.st_size < 1) {
		uint64_t fsiz = HDBHEADSIZ + sizeof(uint64_t) * hbnum;
		if (ftruncate(fd, fsiz) == -1) {
			snprintf(m_emsg, sizeof(m_emsg), "truncate file error, errmsg:[%s]", strerror(errno));
			return ERROR;
		}

		strncpy(m_dbheader->magic, HDBMAGICDATA, sizeof(m_dbheader->magic) - 1);
		m_dbheader->fd = fd;
		m_dbheader->map = map;
		m_dbheader->fsiz = fsiz;
		m_dbheader->bnum = 0; // 块总数
		m_dbheader->fbnum = 0; // 空闲块总数
		m_dbheader->bsiz = bsize > HDBMAXBSIZE ? HDBMAXBSIZE : bsize; // 块大小
		m_dbheader->hbnum = hbnum; // hash桶个数
		m_dbheader->fboff = 0; // 空闲块偏移
		m_dbheader->fmaxsiz = fmaxsiz; // mmap映射空间大小
		m_dbheader->fbaddnum = HDBFBADDNUM;

		// init hash bucket
		int hbsize = sizeof(uint64_t) * m_dbheader->hbnum;
		char * phb = (char *) (map + HDBHEADSIZ);
		memset(phb, 0, hbsize);

		m_prbuf = new char[m_dbheader->fbaddnum * ( sizeof(HDB_BHEADER) + m_dbheader->bsiz )];
		if(!m_prbuf){
			snprintf( m_emsg, sizeof(m_emsg), "new buf error, errmsg:[%s]", strerror(errno) );
			return ERROR;
		}

		// add free block now
		if (!addfb())
			return ERROR;
		sbuf.st_size = m_dbheader->fsiz;
	}

	m_dbheader->fd				= fd;
	m_dbheader->map				= map;
	m_dbheader->fmaxsiz			= fmaxsiz;

	// check
	if ( strncmp( m_dbheader->magic, HDBMAGICDATA, sizeof(m_dbheader->magic) - 1) != 0 || ( (uint64_t)sbuf.st_size != m_dbheader->fsiz )){
		snprintf( m_emsg, sizeof(m_emsg), "check db file error, magic:[%s], size:[%llu|%llu]",
				m_dbheader->magic, m_dbheader->fsiz, sbuf.st_size );
		return ERROR;
	}

	// lock file
	if(lock(fd) == false){
		snprintf( m_emsg, sizeof(m_emsg), "lock db file error, errmsg:[%s]", strerror(errno) );
		return ERROR;
	}

	if(!m_prbuf){
		m_prbuf = new char[m_dbheader->fbaddnum * ( sizeof(HDB_BHEADER) + m_dbheader->bsiz )];
		if(!m_prbuf){
			snprintf( m_emsg, sizeof(m_emsg), "new buf error, errmsg:[%s]", strerror(errno) );
			return ERROR;
		}
	}

	return SUCC;
}

int HDB::get(uint32_t key, char * buf, uint16_t * len ){
	assert(key>=0&&buf&&len);
	//查找节点
	uint32_t idx = hash(key);
	uint64_t boff = *(uint64_t *)(m_dbheader->map + HDBHEADSIZ + idx * sizeof(uint64_t) );
	uint64_t keyoff = 0;
	while(boff > 0){
		HDB_BHEADER * pb = (HDB_BHEADER *)(m_dbheader->map + boff);
		if(key == pb->key){
			keyoff = boff;
			break;
		}
		boff = pb->next;
	}
	if(!keyoff){
		snprintf( m_emsg, sizeof(m_emsg), "key [%u] not found, errmsg:[%s]", key, strerror(errno) );
		return NONODE;
	}

	// 计算长度
	uint16_t bsiz = 0;
	uint64_t doff = keyoff;
	while(doff >0){
		HDB_BHEADER * pb = (HDB_BHEADER *)(m_dbheader->map + doff);
		bsiz += pb->size;
		doff = pb->down;
	}
	if(bsiz > *len){
		snprintf( m_emsg, sizeof(m_emsg), "not enough buf to store data, len=%d, require=%d, errmsg:[%s]", *len, bsiz, strerror(errno) );
		return ERROR;
	}

	//填充数据
	doff=keyoff;
	int off = 0;
	while(doff>0){
		HDB_BHEADER * pb = (HDB_BHEADER *)(m_dbheader->map + doff);
		memcpy(buf+off, pb->data, pb->size);
		off += pb->size;
		doff = pb->down;
	}
	*len = bsiz;

	return SUCC;
}

int HDB::put(uint32_t key, char * buf, uint16_t len){
	assert(key>=0 && buf && len>0);
	uint32_t idx = hash(key);
	uint64_t * pboff = (uint64_t *)(m_dbheader->map + HDBHEADSIZ + idx * sizeof(uint64_t) );

	// 检查节点是否存在
	uint64_t boff = *pboff;
	uint64_t keyoff = 0;
	while(boff > 0){
		HDB_BHEADER * pb = (HDB_BHEADER *)(m_dbheader->map + boff);
		if(key == pb->key){
			keyoff = boff;
			break;
		}
		boff = pb->next;
	}

	// 计算已有块
	vector<uint64_t> vt;
	unsigned int ub = 0;
	uint64_t downoff = keyoff;
	while(downoff > 0){
		HDB_BHEADER * pb = (HDB_BHEADER *)(m_dbheader->map + downoff);
		vt.push_back(downoff);
		downoff = pb->down;
		ub++;
	}

	// 分配额外空间
	unsigned retnum = 0;
	unsigned int addnum = len / m_dbheader->bsiz + 1;
	addnum = addnum > ub ? addnum - ub : 0;
	for(unsigned int i=0; i<addnum; i++){
		uint64_t off;
		if(!getfb(&off)){
			break;
		}
		vt.push_back(off);
		retnum++;
	}

	unsigned tnum = addnum + ub;
	if( vt.size() != tnum ){
		for(unsigned int i=0; i<retnum; i++){
			retfb(vt[ub+i]);
		}
		snprintf( m_emsg, sizeof(m_emsg), "not enough free block now, errmsg:[%s]", strerror(errno) );
		return ERROR;
	}

	// 填充数据
	for(unsigned int i=0; i<tnum; i++){
		HDB_BHEADER * fb = (HDB_BHEADER *)(m_dbheader->map + vt[i]);
		fb->size = len > m_dbheader->bsiz ? m_dbheader->bsiz : len;
		memcpy( fb->data, buf, fb->size);
		buf += fb->size;
		len -= fb->size;
	}

	// 内部链接
	for(unsigned int i=0; i<tnum - 1; i++){
		HDB_BHEADER * fb = (HDB_BHEADER *)(m_dbheader->map + vt[i]);
		fb->down = vt[i+1];
	}
	HDB_BHEADER * end = (HDB_BHEADER *)(m_dbheader->map + vt[vt.size()-1]);
	end->down = 0;

	// 挂到hash bucket上
	if(!keyoff){
		HDB_BHEADER * fstfb = (HDB_BHEADER *)(m_dbheader->map + vt[0]);
		fstfb->key = key;
		uint64_t boff = *pboff;
		*pboff = vt[0];
		fstfb->next = boff;
		fstfb->pre = 0;
		if(boff){
			HDB_BHEADER * bfb = (HDB_BHEADER *)(m_dbheader->map + boff);
			bfb->pre = vt[0];
		}
	}

	return SUCC;
}

int HDB::out(uint32_t key){
	assert(key>=0);
	// 查找节点
	uint32_t idx = hash(key);
	uint64_t boff = *(uint64_t *)(m_dbheader->map + HDBHEADSIZ + idx * sizeof(uint64_t) );
	uint64_t keyoff = 0;
	while(boff > 0){
		HDB_BHEADER * pb = (HDB_BHEADER *)(m_dbheader->map + boff);
		if(key == pb->key){
			keyoff = boff;
			break;
		}
		boff = pb->next;
	}
	if(!keyoff){
		snprintf( m_emsg, sizeof(m_emsg), "key [%u] not found, errmsg:[%s]", key, strerror(errno) );
		return NONODE;
	}

	// 断开链接
	HDB_BHEADER * keynode = (HDB_BHEADER *)(m_dbheader->map + keyoff);
	if(keynode->pre){
		HDB_BHEADER * pnode = (HDB_BHEADER *)(m_dbheader->map + keynode->pre);
		pnode->next = keynode->next;
		if(keynode->next){
			HDB_BHEADER * nnode = (HDB_BHEADER *)(m_dbheader->map + keynode->next);
			nnode->pre = keynode->pre;
		}
	}else{
		uint64_t * poff = (uint64_t *)(m_dbheader->map + HDBHEADSIZ + idx * sizeof(uint64_t) );
		*poff = keynode->next;
		if(keynode->next){
			HDB_BHEADER * nnode = (HDB_BHEADER *)(m_dbheader->map + keynode->next);
			nnode->pre = keynode->pre;
		}
	}

	// 回收节点
	uint64_t downoff = keyoff;
	while(downoff > 0){
		HDB_BHEADER * pb = (HDB_BHEADER *)(m_dbheader->map + downoff);
		uint64_t tmp = pb->down;
		retfb(downoff);
		downoff = tmp;
	}

	return SUCC;
}

bool HDB::getfb(uint64_t *poff){
	if(!m_dbheader->fboff){
		if(!addfb()) return false;
	}

	uint64_t hoff = m_dbheader->fboff;
	HDB_BHEADER * fb = (HDB_BHEADER *)(m_dbheader->map + m_dbheader->fboff);
	m_dbheader->fboff = fb->next;
	if(fb->next){
		HDB_BHEADER * fbn = (HDB_BHEADER *)(m_dbheader->map + fb->next);
		fbn->pre = 0;
	}
	m_dbheader->fbnum--;
	*poff = hoff;
	return true;
}

bool HDB::addfb(){
	size_t sz = m_dbheader->fbaddnum * ( sizeof(HDB_BHEADER) + m_dbheader->bsiz );
	uint64_t off = m_dbheader->fsiz;

	if( !seekwrite( off, m_prbuf, sz ) ) return false;

	for(int i=0; i<m_dbheader->fbaddnum; i++){
		uint64_t fboff = off + i * ( sizeof(HDB_BHEADER) + m_dbheader->bsiz );
		retfb(fboff);
		m_dbheader->bnum++;
	}

	return true;
}

bool HDB::retfb(uint64_t off){
	assert(off > 0);
	HDB_BHEADER * pfb = (HDB_BHEADER *)(m_dbheader->map + off);
	HDB_BHEADER * pfbnow = (HDB_BHEADER *)(m_dbheader->map + m_dbheader->fboff);

	pfb->key = 0;
	pfb->size = 0;
	pfb->down = 0;
	uint64_t noff = m_dbheader->fboff;
	m_dbheader->fboff = off;
	pfb->next = noff;
	if(noff) pfbnow->pre = off;
	pfb->pre = 0;
	m_dbheader->fbnum++;

	return true;
}

// Thomas Wang的算法，整数hash
uint32_t HDB::hash(uint32_t key){
	key += ~(key << 15);
	key ^= (key >> 10);
	key += (key << 3);
	key ^= (key >> 6);
	key += ~(key << 11);
	key ^= (key >> 16);
	return key % m_dbheader->hbnum;
}

/* Lock a file. */
bool HDB::lock(int fd) {
	assert(fd >= 0);
	struct flock lock;
	memset(&lock, 0, sizeof(struct flock));
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_pid = getpid();
	while (fcntl(fd, F_SETLKW, &lock) == -1) {
		if (errno != EINTR)
			return false;
	}
	return true;
}

/* Unlock a file. */
bool HDB::unlock(int fd) {
	assert(fd >= 0);
	struct flock lock;
	memset(&lock, 0, sizeof(struct flock));
	lock.l_type = F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_pid = 0;
	while (fcntl(fd, F_SETLKW, &lock) == -1) {
		if (errno != EINTR)
			return false;
	}
	return true;
}

bool HDB::seekwrite(uint64_t off, const void *buf, size_t size) {
	assert(m_dbheader->fd != 0 && off >= 0 && buf && size >= 0);
	uint64_t end = off + size;
	if (end <= m_dbheader->fmaxsiz) {
		if (end > m_dbheader->fsiz) {
			uint64_t fsiz = end;
			if (ftruncate(m_dbheader->fd, fsiz) == -1) {
				snprintf( m_emsg, sizeof(m_emsg), "seekwrite truncate error, errmsg:[%s]", strerror(errno) );
				return false;
			}
			m_dbheader->fsiz = fsiz;
		}
		memcpy(m_dbheader->map + off, buf, size);
		return true;
	}else{
		snprintf( m_emsg, sizeof(m_emsg), "seekwrite error, not enough space, errmsg:[%s]", strerror(errno) );
		return false;
	}
}

bool HDB::seekread(uint64_t off, void *buf, size_t size) {
	assert( m_dbheader->fd != 0 && off >= 0 && buf && size >= 0);
	if (off + size <= m_dbheader->fmaxsiz) {
		memcpy(buf, m_dbheader->map + off, size);
		return true;
	}else{
		snprintf( m_emsg, sizeof(m_emsg), "seekread error, off + size too big, errmsg:[%s]", strerror(errno) );
		return false;
	}
}










