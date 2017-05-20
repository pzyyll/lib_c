/*************************************************************************************************
 * The hash database API of Tokyo Cabinet
 *                                                      Copyright (C) 2006-2010 Mikio Hirabayashi
 * This file is part of Tokyo Cabinet.
 * Tokyo Cabinet is free software; you can redistribute it and/or modify it under the terms of
 * the GNU Lesser General Public License as published by the Free Software Foundation; either
 * version 2.1 of the License or any later version.  Tokyo Cabinet is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 * You should have received a copy of the GNU Lesser General Public License along with Tokyo
 * Cabinet; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA.
 *************************************************************************************************/

/**
 * ChangeLog
 * 20100331 jamieli 接口中去除如下功能
 *   1)tchdbfwmkeys相关功能函数，暂时没有DB搜索的需要，就算需要搜索，也不会整个DB遍历
 *   2)tchdbtran相关的功能函数，暂时没有关于事务的需求，同时该事务实现方式是排他的，只要有事务存在，其他的所有线程不能对DB进行操作
 *   3)tchdbputproc暂时没有该需求
 *   4)tchdbgetnext相关功能函数，暂时没有该需求，同时该功能在使用过程中，使用KEY作为iter当前位置的依据，同时KEY对应的节点有可能发生变化，导致该功能使用会出问题
 *   5)tchdbforeach函数，该函数在扫描回调期间，会锁住所有的记录，其他线程将不能操作DB
 *
 *
 *
 * TODO
 * 1)需要增加文件扩展大小限制
 * 2)需要修改碎片整理时的一个BUG
 */

#ifndef _TCHDB_H                         /* duplication check */
#define _TCHDB_H

#if defined(__cplusplus)
#define __TCHDB_CLINKAGEBEGIN extern "C" {
#define __TCHDB_CLINKAGEEND }
#else
#define __TCHDB_CLINKAGEBEGIN
#define __TCHDB_CLINKAGEEND
#endif
__TCHDB_CLINKAGEBEGIN


#include "comm/hash_db/tcutil.h"


/*************************************************************************************************
 * API
 *************************************************************************************************/


typedef struct {                         /* type of structure for a hash database */
  void *mmtx;                            /* mutex for method */
  void *rmtxs;                           /* mutexes for records */
  void *dmtx;                            /* mutex for the while database */
  void *wmtx;                            /* mutex for write ahead logging */
  void *eckey;                           /* key for thread specific error code */
  char *rpath;                           /* real path for locking */
  uint8_t type;                          /* database type */
  uint8_t flags;                         /* additional flags */
  uint64_t bnum;                         /* number of the bucket array */
  uint8_t apow;                          /* power of record alignment */
  uint8_t fpow;                          /* power of free block pool number */
  uint8_t opts;                          /* options */
  char *path;                            /* path of the database file */
  int fd;                                /* file descriptor of the database file */
  uint32_t omode;                        /* open mode */
  uint64_t rnum;                         /* number of the records */
  uint64_t fsiz;                         /* size of the database file */
  uint64_t maxfsiz;                      /* Maxsize of the database file */
  uint64_t frec;                         /* offset of the first record */
  uint64_t dfcur;                        /* offset of the cursor for defragmentation */
  uint64_t iter;                         /* offset of the iterator */
  char *map;                             /* pointer to the mapped memory */
  uint64_t msiz;                         /* size of the mapped memory */
  uint64_t xmsiz;                        /* size of the extra mapped memory */
  uint64_t xfsiz;                        /* extra size of the file for mapped memory */
  uint32_t *ba32;                        /* 32-bit bucket array */
  uint64_t *ba64;                        /* 64-bit bucket array */
  uint32_t align;                        /* record alignment */
  uint32_t runit;                        /* record reading unit */
  bool zmode;                            /* whether compression is used */
  int32_t fbpmax;                        /* maximum number of the free block pool */
  void *fbpool;                          /* free block pool */
  int32_t fbpnum;                        /* number of the free block pool */
  int32_t fbpmis;                        /* number of missing retrieval of the free block pool */
  bool async;                            /* whether asynchronous storing is called */
  TCXSTR *drpool;                        /* delayed record pool */
  TCXSTR *drpdef;                        /* deferred records of the delayed record pool */
  uint64_t drpoff;                       /* offset of the delayed record pool */
  TCMDB *recc;                           /* cache for records */
  uint32_t rcnum;                        /* maximum number of cached records */
  TCCODEC enc;                           /* pointer to the encoding function */
  void *encop;                           /* opaque object for the encoding functions */
  TCCODEC dec;                           /* pointer to the decoding function */
  void *decop;                           /* opaque object for the decoding functions */
  int ecode;                             /* last happened error code */
  bool fatal;                            /* whether a fatal error occured */
  uint64_t inode;                        /* inode number */
  time_t mtime;                          /* modification time */
  uint32_t dfunit;                       /* unit step number of auto defragmentation */
  uint32_t dfcnt;                        /* counter of auto defragmentation */  //自动碎片整理计数器
  bool tran;                             /* whether in the transaction */
  int walfd;                             /* file descriptor of write ahead logging */
  uint64_t walend;                       /* end offset of write ahead logging */
  int dbgfd;                             /* file descriptor for debugging */
  volatile int64_t cnt_writerec;         /* tesing counter for record write times */
  volatile int64_t cnt_reuserec;         /* tesing counter for record reuse times */
  volatile int64_t cnt_moverec;          /* tesing counter for record move times */
  volatile int64_t cnt_readrec;          /* tesing counter for record read times */
  volatile int64_t cnt_searchfbp;        /* tesing counter for FBP search times */
  volatile int64_t cnt_insertfbp;        /* tesing counter for FBP insert times */
  volatile int64_t cnt_splicefbp;        /* tesing counter for FBP splice times */
  volatile int64_t cnt_dividefbp;        /* tesing counter for FBP divide times */
  volatile int64_t cnt_mergefbp;         /* tesing counter for FBP merge times */
  volatile int64_t cnt_reducefbp;        /* tesing counter for FBP reduce times */
  volatile int64_t cnt_appenddrp;        /* tesing counter for DRP append times */
  volatile int64_t cnt_deferdrp;         /* tesing counter for DRP defer times */
  volatile int64_t cnt_flushdrp;         /* tesing counter for DRP flush times */
  volatile int64_t cnt_adjrecc;          /* tesing counter for record cache adjust times */
  volatile int64_t cnt_defrag;           /* tesing counter for defragmentation times */
  volatile int64_t cnt_shiftrec;         /* tesing counter for record shift times */
  volatile int64_t cnt_trunc;            /* tesing counter for truncation times */
} TCHDB;

enum {                                   /* enumeration for additional flags */
  HDBFOPEN = 1 << 0,                     /* whether opened */
  HDBFFATAL = 1 << 1                     /* whether with fatal error */
};

enum {                                   /* enumeration for tuning options */
  HDBTLARGE = 1 << 0,                    /* use 64-bit bucket array */
  HDBTDEFLATE = 1 << 1,                  /* compress each record with Deflate */
  HDBTBZIP = 1 << 2,                     /* compress each record with BZIP2 */
  HDBTTCBS = 1 << 3,                     /* compress each record with TCBS */
  HDBTEXCODEC = 1 << 4                   /* compress each record with custom functions */
};

enum {                                   /* enumeration for open modes */
  HDBOREADER = 1 << 0,                   /* open as a reader */
  HDBOWRITER = 1 << 1,                   /* open as a writer */
  HDBOCREAT = 1 << 2,                    /* writer creating */
  HDBOTRUNC = 1 << 3,                    /* writer truncating */
  HDBONOLCK = 1 << 4,                    /* open without locking */
  HDBOLCKNB = 1 << 5,                    /* lock without blocking */
  HDBOTSYNC = 1 << 6                     /* synchronize every transaction */
};

typedef struct {                         // type of structure for a record
  uint64_t off;                          // offset of the record
  uint32_t rsiz;                         // size of the whole record
  uint8_t magic;                         // magic number
  uint8_t hash;                          // second hash value
  uint64_t left;                         // offset of the left child record
  uint64_t right;                        // offset of the right child record
  uint32_t ksiz;                         // size of the key
  uint32_t vsiz;                         // size of the value
  uint16_t psiz;                         // size of the padding
  const char *kbuf;                      // pointer to the key
  const char *vbuf;                      // pointer to the value
  uint64_t boff;                         // offset of the body
  char *bbuf;                            // buffer of the body
} TCHREC;

typedef struct {                         // type of structure for a free block
  uint64_t off;                          // offset of the block
  uint32_t rsiz;                         // size of the block
} HDBFB;


/* Get the message string corresponding to an error code.
   `ecode' specifies the error code.
   The return value is the message string of the error code. */
const char *tchdberrmsg(int ecode);


/* Create a hash database object.
   The return value is the new hash database object. */
TCHDB *tchdbnew(void);


/* Delete a hash database object.
   `hdb' specifies the hash database object.
   If the database is not closed, it is closed implicitly.  Note that the deleted object and its
   derivatives can not be used anymore. */
void tchdbdel(TCHDB *hdb);


/* Get the last happened error code of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the last happened error code.
   The following error codes are defined: `TCESUCCESS' for success, `TCETHREAD' for threading
   error, `TCEINVALID' for invalid operation, `TCENOFILE' for file not found, `TCENOPERM' for no
   permission, `TCEMETA' for invalid meta data, `TCERHEAD' for invalid record header, `TCEOPEN'
   for open error, `TCECLOSE' for close error, `TCETRUNC' for trunc error, `TCESYNC' for sync
   error, `TCESTAT' for stat error, `TCESEEK' for seek error, `TCEREAD' for read error,
   `TCEWRITE' for write error, `TCEMMAP' for mmap error, `TCELOCK' for lock error, `TCEUNLINK'
   for unlink error, `TCERENAME' for rename error, `TCEMKDIR' for mkdir error, `TCERMDIR' for
   rmdir error, `TCEKEEP' for existing record, `TCENOREC' for no record found, and `TCEMISC' for
   miscellaneous error. */
int tchdbecode(TCHDB *hdb);


/* Set mutual exclusion control of a hash database object for threading.
   `hdb' specifies the hash database object which is not opened.
   If successful, the return value is true, else, it is false.
   Note that the mutual exclusion control is needed if the object is shared by plural threads and
   this function should be called before the database is opened. */
bool tchdbsetmutex(TCHDB *hdb);


/* Set the tuning parameters of a hash database object.
   `hdb' specifies the hash database object which is not opened.
   `bnum' specifies the number of elements of the bucket array.  If it is not more than 0, the
   default value is specified.  The default value is 131071.  Suggested size of the bucket array
   is about from 0.5 to 4 times of the number of all records to be stored.
   `apow' specifies the size of record alignment by power of 2.  If it is negative, the default
   value is specified.  The default value is 4 standing for 2^4=16.
   `fpow' specifies the maximum number of elements of the free block pool by power of 2.  If it
   is negative, the default value is specified.  The default value is 10 standing for 2^10=1024.
   `opts' specifies options by bitwise-or: `HDBTLARGE' specifies that the size of the database
   can be larger than 2GB by using 64-bit bucket array, `HDBTDEFLATE' specifies that each record
   is compressed with Deflate encoding, `HDBTBZIP' specifies that each record is compressed with
   BZIP2 encoding, `HDBTTCBS' specifies that each record is compressed with TCBS encoding.
   If successful, the return value is true, else, it is false.
   Note that the tuning parameters should be set before the database is opened. */
bool tchdbtune(TCHDB *hdb, int64_t bnum, int8_t apow, int8_t fpow, uint8_t opts);


/* Set the caching parameters of a hash database object.
   `hdb' specifies the hash database object which is not opened.
   `rcnum' specifies the maximum number of records to be cached.  If it is not more than 0, the
   record cache is disabled.  It is disabled by default.
   If successful, the return value is true, else, it is false.
   Note that the caching parameters should be set before the database is opened. */
bool tchdbsetcache(TCHDB *hdb, int32_t rcnum);


/* Set the size of the extra mapped memory of a hash database object.
   `hdb' specifies the hash database object which is not opened.
   `xmsiz' specifies the size of the extra mapped memory.  If it is not more than 0, the extra
   mapped memory is disabled.  The default size is 67108864.
   If successful, the return value is true, else, it is false.
   Note that the mapping parameters should be set before the database is opened. */
bool tchdbsetxmsiz(TCHDB *hdb, int64_t xmsiz);

/* Set the unit step number of auto defragmentation of a hash database object.
   `hdb' specifies the hash database object which is not opened.
   `dfunit' specifie the unit step number.  If it is not more than 0, the auto defragmentation
   is disabled.  It is disabled by default.
   If successful, the return value is true, else, it is false.
   Note that the defragmentation parameters should be set before the database is opened. */
bool tchdbsetdfunit(TCHDB *hdb, int32_t dfunit);


/* Open a database file and connect a hash database object.
   `hdb' specifies the hash database object which is not opened.
   `path' specifies the path of the database file.
   `omode' specifies the connection mode: `HDBOWRITER' as a writer, `HDBOREADER' as a reader.
   If the mode is `HDBOWRITER', the following may be added by bitwise-or: `HDBOCREAT', which
   means it creates a new database if not exist, `HDBOTRUNC', which means it creates a new
   database regardless if one exists, `HDBOTSYNC', which means every transaction synchronizes
   updated contents with the device.  Both of `HDBOREADER' and `HDBOWRITER' can be added to by
   bitwise-or: `HDBONOLCK', which means it opens the database file without file locking, or
   `HDBOLCKNB', which means locking is performed without blocking.
   If successful, the return value is true, else, it is false. */
bool tchdbopen(TCHDB *hdb, const char *path, int omode);


/* Close a hash database object.
   `hdb' specifies the hash database object.
   If successful, the return value is true, else, it is false.
   Update of a database is assured to be written when the database is closed.  If a writer opens
   a database but does not close it appropriately, the database will be broken. */
bool tchdbclose(TCHDB *hdb);


/* Store a record into a hash database object.
   `hdb' specifies the hash database object connected as a writer.
   `kbuf' specifies the pointer to the region of the key.
   `ksiz' specifies the size of the region of the key.
   `vbuf' specifies the pointer to the region of the value.
   `vsiz' specifies the size of the region of the value.
   If successful, the return value is true, else, it is false.
   If a record with the same key exists in the database, it is overwritten. */
bool tchdbput(TCHDB *hdb, const void *kbuf, int ksiz, const void *vbuf, int vsiz);


/* Store a string record into a hash database object.
   `hdb' specifies the hash database object connected as a writer.
   `kstr' specifies the string of the key.
   `vstr' specifies the string of the value.
   If successful, the return value is true, else, it is false.
   If a record with the same key exists in the database, it is overwritten. */
bool tchdbput2(TCHDB *hdb, const char *kstr, const char *vstr);


/* Store a new record into a hash database object.
   `hdb' specifies the hash database object connected as a writer.
   `kbuf' specifies the pointer to the region of the key.
   `ksiz' specifies the size of the region of the key.
   `vbuf' specifies the pointer to the region of the value.
   `vsiz' specifies the size of the region of the value.
   If successful, the return value is true, else, it is false.
   If a record with the same key exists in the database, this function has no effect. */
bool tchdbputkeep(TCHDB *hdb, const void *kbuf, int ksiz, const void *vbuf, int vsiz);


/* Store a new string record into a hash database object.
   `hdb' specifies the hash database object connected as a writer.
   `kstr' specifies the string of the key.
   `vstr' specifies the string of the value.
   If successful, the return value is true, else, it is false.
   If a record with the same key exists in the database, this function has no effect. */
bool tchdbputkeep2(TCHDB *hdb, const char *kstr, const char *vstr);


/* Concatenate a value at the end of the existing record in a hash database object.
   `hdb' specifies the hash database object connected as a writer.
   `kbuf' specifies the pointer to the region of the key.
   `ksiz' specifies the size of the region of the key.
   `vbuf' specifies the pointer to the region of the value.
   `vsiz' specifies the size of the region of the value.
   If successful, the return value is true, else, it is false.
   If there is no corresponding record, a new record is created. */
bool tchdbputcat(TCHDB *hdb, const void *kbuf, int ksiz, const void *vbuf, int vsiz);


/* Concatenate a string value at the end of the existing record in a hash database object.
   `hdb' specifies the hash database object connected as a writer.
   `kstr' specifies the string of the key.
   `vstr' specifies the string of the value.
   If successful, the return value is true, else, it is false.
   If there is no corresponding record, a new record is created. */
bool tchdbputcat2(TCHDB *hdb, const char *kstr, const char *vstr);


/* Store a record into a hash database object in asynchronous fashion.
   `hdb' specifies the hash database object connected as a writer.
   `kbuf' specifies the pointer to the region of the key.
   `ksiz' specifies the size of the region of the key.
   `vbuf' specifies the pointer to the region of the value.
   `vsiz' specifies the size of the region of the value.
   If successful, the return value is true, else, it is false.
   If a record with the same key exists in the database, it is overwritten.  Records passed to
   this function are accumulated into the inner buffer and wrote into the file at a blast. */
bool tchdbputasync(TCHDB *hdb, const void *kbuf, int ksiz, const void *vbuf, int vsiz);


/* Store a string record into a hash database object in asynchronous fashion.
   `hdb' specifies the hash database object connected as a writer.
   `kstr' specifies the string of the key.
   `vstr' specifies the string of the value.
   If successful, the return value is true, else, it is false.
   If a record with the same key exists in the database, it is overwritten.  Records passed to
   this function are accumulated into the inner buffer and wrote into the file at a blast. */
bool tchdbputasync2(TCHDB *hdb, const char *kstr, const char *vstr);


/* Remove a record of a hash database object.
   `hdb' specifies the hash database object connected as a writer.
   `kbuf' specifies the pointer to the region of the key.
   `ksiz' specifies the size of the region of the key.
   If successful, the return value is true, else, it is false. */
bool tchdbout(TCHDB *hdb, const void *kbuf, int ksiz);


/* Remove a string record of a hash database object.
   `hdb' specifies the hash database object connected as a writer.
   `kstr' specifies the string of the key.
   If successful, the return value is true, else, it is false. */
bool tchdbout2(TCHDB *hdb, const char *kstr);


/* Retrieve a record in a hash database object.
   `hdb' specifies the hash database object.
   `kbuf' specifies the pointer to the region of the key.
   `ksiz' specifies the size of the region of the key.
   `sp' specifies the pointer to the variable into which the size of the region of the return
   value is assigned.
   If successful, the return value is the pointer to the region of the value of the corresponding
   record.  `NULL' is returned if no record corresponds.
   Because an additional zero code is appended at the end of the region of the return value,
   the return value can be treated as a character string.  Because the region of the return
   value is allocated with the `malloc' call, it should be released with the `free' call when
   it is no longer in use. */
void *tchdbget(TCHDB *hdb, const void *kbuf, int ksiz, int *sp);


/* Retrieve a string record in a hash database object.
   `hdb' specifies the hash database object.
   `kstr' specifies the string of the key.
   If successful, the return value is the string of the value of the corresponding record.
   `NULL' is returned if no record corresponds.
   Because the region of the return value is allocated with the `malloc' call, it should be
   released with the `free' call when it is no longer in use. */
char *tchdbget2(TCHDB *hdb, const char *kstr);


/* Retrieve a record in a hash database object and write the value into a buffer.
   `hdb' specifies the hash database object.
   `kbuf' specifies the pointer to the region of the key.
   `ksiz' specifies the size of the region of the key.
   `vbuf' specifies the pointer to the buffer into which the value of the corresponding record is
   written.
   `max' specifies the size of the buffer.
   If successful, the return value is the size of the written data, else, it is -1.  -1 is
   returned if no record corresponds to the specified key.
   Note that an additional zero code is not appended at the end of the region of the writing
   buffer. */
int tchdbget3(TCHDB *hdb, const void *kbuf, int ksiz, void *vbuf, int max);


/* Get the size of the value of a record in a hash database object.
   `hdb' specifies the hash database object.
   `kbuf' specifies the pointer to the region of the key.
   `ksiz' specifies the size of the region of the key.
   If successful, the return value is the size of the value of the corresponding record, else,
   it is -1. */
int tchdbvsiz(TCHDB *hdb, const void *kbuf, int ksiz);


/* Get the size of the value of a string record in a hash database object.
   `hdb' specifies the hash database object.
   `kstr' specifies the string of the key.
   If successful, the return value is the size of the value of the corresponding record, else,
   it is -1. */
int tchdbvsiz2(TCHDB *hdb, const char *kstr);


/* Initialize the iterator of a hash database object.
   `hdb' specifies the hash database object.
   If successful, the return value is true, else, it is false.
   The iterator is used in order to access the key of every record stored in a database. */
bool tchdbiterinit(TCHDB *hdb);


/* Get the next key of the iterator of a hash database object.
   `hdb' specifies the hash database object.
   `sp' specifies the pointer to the variable into which the size of the region of the return
   value is assigned.
   If successful, the return value is the pointer to the region of the next key, else, it is
   `NULL'.  `NULL' is returned when no record is to be get out of the iterator.
   Because an additional zero code is appended at the end of the region of the return value, the
   return value can be treated as a character string.  Because the region of the return value is
   allocated with the `malloc' call, it should be released with the `free' call when it is no
   longer in use.  It is possible to access every record by iteration of calling this function.
   It is allowed to update or remove records whose keys are fetched while the iteration.
   However, it is not assured if updating the database is occurred while the iteration.  Besides,
   the order of this traversal access method is arbitrary, so it is not assured that the order of
   storing matches the one of the traversal access. */
void *tchdbiternext(TCHDB *hdb, int *sp);


/* Get the next key string of the iterator of a hash database object.
   `hdb' specifies the hash database object.
   If successful, the return value is the string of the next key, else, it is `NULL'.  `NULL' is
   returned when no record is to be get out of the iterator.
   Because the region of the return value is allocated with the `malloc' call, it should be
   released with the `free' call when it is no longer in use.  It is possible to access every
   record by iteration of calling this function.  However, it is not assured if updating the
   database is occurred while the iteration.  Besides, the order of this traversal access method
   is arbitrary, so it is not assured that the order of storing matches the one of the traversal
   access. */
char *tchdbiternext2(TCHDB *hdb);


/* Get the next extensible objects of the iterator of a hash database object.
   `hdb' specifies the hash database object.
   `kxstr' specifies the object into which the next key is wrote down.
   `vxstr' specifies the object into which the next value is wrote down.
   If successful, the return value is true, else, it is false.  False is returned when no record
   is to be get out of the iterator. */
bool tchdbiternext3(TCHDB *hdb, TCXSTR *kxstr, TCXSTR *vxstr);


/* Add an integer to a record in a hash database object.
   `hdb' specifies the hash database object connected as a writer.
   `kbuf' specifies the pointer to the region of the key.
   `ksiz' specifies the size of the region of the key.
   `num' specifies the additional value.
   If successful, the return value is the summation value, else, it is `INT_MIN'.
   If the corresponding record exists, the value is treated as an integer and is added to.  If no
   record corresponds, a new record of the additional value is stored. */
int tchdbaddint(TCHDB *hdb, const void *kbuf, int ksiz, int num);


/* Add a real number to a record in a hash database object.
   `hdb' specifies the hash database object connected as a writer.
   `kbuf' specifies the pointer to the region of the key.
   `ksiz' specifies the size of the region of the key.
   `num' specifies the additional value.
   If successful, the return value is the summation value, else, it is Not-a-Number.
   If the corresponding record exists, the value is treated as a real number and is added to.  If
   no record corresponds, a new record of the additional value is stored. */
double tchdbadddouble(TCHDB *hdb, const void *kbuf, int ksiz, double num);


/* Synchronize updated contents of a hash database object with the file and the device.
   `hdb' specifies the hash database object connected as a writer.
   If successful, the return value is true, else, it is false.
   This function is useful when another process connects to the same database file. */
bool tchdbsync(TCHDB *hdb);


/* Optimize the file of a hash database object.
   `hdb' specifies the hash database object connected as a writer.
   `bnum' specifies the number of elements of the bucket array.  If it is not more than 0, the
   default value is specified.  The default value is two times of the number of records.
   `apow' specifies the size of record alignment by power of 2.  If it is negative, the current
   setting is not changed.
   `fpow' specifies the maximum number of elements of the free block pool by power of 2.  If it
   is negative, the current setting is not changed.
   `opts' specifies options by bitwise-or: `HDBTLARGE' specifies that the size of the database
   can be larger than 2GB by using 64-bit bucket array, `HDBTDEFLATE' specifies that each record
   is compressed with Deflate encoding, `HDBTBZIP' specifies that each record is compressed with
   BZIP2 encoding, `HDBTTCBS' specifies that each record is compressed with TCBS encoding.  If it
   is `UINT8_MAX', the current setting is not changed.
   If successful, the return value is true, else, it is false.
   This function is useful to reduce the size of the database file with data fragmentation by
   successive updating. */
bool tchdboptimize(TCHDB *hdb, int64_t bnum, int8_t apow, int8_t fpow, uint8_t opts);


/* Remove all records of a hash database object.
   `hdb' specifies the hash database object connected as a writer.
   If successful, the return value is true, else, it is false. */
bool tchdbvanish(TCHDB *hdb);


/* Copy the database file of a hash database object.
   `hdb' specifies the hash database object.
   `path' specifies the path of the destination file.  If it begins with `@', the trailing
   substring is executed as a command line.
   If successful, the return value is true, else, it is false.  False is returned if the executed
   command returns non-zero code.
   The database file is assured to be kept synchronized and not modified while the copying or
   executing operation is in progress.  So, this function is useful to create a backup file of
   the database file. */
bool tchdbcopy(TCHDB *hdb, const char *path);

/* Get the file path of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the path of the database file or `NULL' if the object does not connect to
   any database file. */
const char *tchdbpath(TCHDB *hdb);


/* Get the number of records of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the number of records or 0 if the object does not connect to any database
   file. */
uint64_t tchdbrnum(TCHDB *hdb);


/* Get the size of the database file of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the size of the database file or 0 if the object does not connect to any
   database file. */
uint64_t tchdbfsiz(TCHDB *hdb);



/*************************************************************************************************
 * features for experts
 *************************************************************************************************/


/* Set the error code of a hash database object.
   `hdb' specifies the hash database object.
   `ecode' specifies the error code.
   `file' specifies the file name of the code.
   `line' specifies the line number of the code.
   `func' specifies the function name of the code. */
void tchdbsetecode(TCHDB *hdb, int ecode, const char *filename, int line, const char *func);


/* Set the type of a hash database object.
   `hdb' specifies the hash database object.
   `type' specifies the database type. */
void tchdbsettype(TCHDB *hdb, uint8_t type);


/* Set the file descriptor for debugging output.
   `hdb' specifies the hash database object.
   `fd' specifies the file descriptor for debugging output. */
void tchdbsetdbgfd(TCHDB *hdb, int fd);


/* Get the file descriptor for debugging output.
   `hdb' specifies the hash database object.
   The return value is the file descriptor for debugging output. */
int tchdbdbgfd(TCHDB *hdb);


/* Check whether mutual exclusion control is set to a hash database object.
   `hdb' specifies the hash database object.
   If mutual exclusion control is set, it is true, else it is false. */
bool tchdbhasmutex(TCHDB *hdb);


/* Synchronize updating contents on memory of a hash database object.
   `hdb' specifies the hash database object connected as a writer.
   `phys' specifies whether to synchronize physically.
   If successful, the return value is true, else, it is false. */
bool tchdbmemsync(TCHDB *hdb, bool phys);


/* Get the number of elements of the bucket array of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the number of elements of the bucket array or 0 if the object does not
   connect to any database file. */
uint64_t tchdbbnum(TCHDB *hdb);


/* Get the record alignment of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the record alignment or 0 if the object does not connect to any database
   file. */
uint32_t tchdbalign(TCHDB *hdb);


/* Get the maximum number of the free block pool of a a hash database object.
   `hdb' specifies the hash database object.
   The return value is the maximum number of the free block pool or 0 if the object does not
   connect to any database file. */
uint32_t tchdbfbpmax(TCHDB *hdb);


/* Get the size of the extra mapped memory of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the size of the extra mapped memory or 0 if the object does not connect to
   any database file. */
uint64_t tchdbxmsiz(TCHDB *hdb);


/* Get the inode number of the database file of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the inode number of the database file or 0 if the object does not connect
   to any database file. */
uint64_t tchdbinode(TCHDB *hdb);


/* Get the modification time of the database file of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the inode number of the database file or 0 if the object does not connect
   to any database file. */
time_t tchdbmtime(TCHDB *hdb);


/* Get the connection mode of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the connection mode. */
int tchdbomode(TCHDB *hdb);


/* Get the database type of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the database type. */
uint8_t tchdbtype(TCHDB *hdb);


/* Get the additional flags of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the additional flags. */
uint8_t tchdbflags(TCHDB *hdb);


/* Get the options of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the options. */
uint8_t tchdbopts(TCHDB *hdb);


/* Get the pointer to the opaque field of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the pointer to the opaque field whose size is 128 bytes. */
char *tchdbopaque(TCHDB *hdb);


/* Get the number of used elements of the bucket array of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the number of used elements of the bucket array or 0 if the object does
   not connect to any database file. */
uint64_t tchdbbnumused(TCHDB *hdb);


/* Set the custom codec functions of a hash database object.
   `hdb' specifies the hash database object.
   `enc' specifies the pointer to the custom encoding function.  It receives four parameters.
   The first parameter is the pointer to the region.  The second parameter is the size of the
   region.  The third parameter is the pointer to the variable into which the size of the region
   of the return value is assigned.  The fourth parameter is the pointer to the optional opaque
   object.  It returns the pointer to the result object allocated with `malloc' call if
   successful, else, it returns `NULL'.
   `encop' specifies an arbitrary pointer to be given as a parameter of the encoding function.
   If it is not needed, `NULL' can be specified.
   `dec' specifies the pointer to the custom decoding function.
   `decop' specifies an arbitrary pointer to be given as a parameter of the decoding function.
   If it is not needed, `NULL' can be specified.
   If successful, the return value is true, else, it is false.
   Note that the custom codec functions should be set before the database is opened and should be
   set every time the database is being opened. */
bool tchdbsetcodecfunc(TCHDB *hdb, TCCODEC enc, void *encop, TCCODEC dec, void *decop);


/* Get the custom codec functions of a hash database object.
   `hdb' specifies the hash database object.
   `ep' specifies the pointer to a variable into which the pointer to the custom encoding
   function is assigned
   `eop' specifies the pointer to a variable into which the arbitrary pointer to be given to the
   encoding function is assigned.
   `dp' specifies the pointer to a variable into which the pointer to the custom decoding
   function is assigned
   `dop' specifies the pointer to a variable into which the arbitrary pointer to be given to the
   decoding function is assigned. */
void tchdbcodecfunc(TCHDB *hdb, TCCODEC *ep, void **eop, TCCODEC *dp, void **dop);


/* Get the unit step number of auto defragmentation of a hash database object.
   `hdb' specifies the hash database object.
   The return value is the unit step number of auto defragmentation. */
uint32_t tchdbdfunit(TCHDB *hdb);


/* Perform dynamic defragmentation of a hash database object.
   `hdb' specifies the hash database object connected as a writer.
   `step' specifie the number of steps.  If it is not more than 0, the whole file is defragmented
   gradually without keeping a continuous lock.
   If successful, the return value is true, else, it is false. */
bool tchdbdefrag(TCHDB *hdb, int64_t step);


/* Clear the cache of a hash tree database object.
   `hdb' specifies the hash tree database object.
   If successful, the return value is true, else, it is false. */
bool tchdbcacheclear(TCHDB *hdb);


/* Move the iterator to the record corresponding a key of a hash database object.
   `hdb' specifies the hash database object.
   `kbuf' specifies the pointer to the region of the key.
   `ksiz' specifies the size of the region of the key.
   If successful, the return value is true, else, it is false.  False is returned if there is
   no record corresponding the condition. */
bool tchdbiterinit2(TCHDB *hdb, const void *kbuf, int ksiz);


/* Move the iterator to the record corresponding a key string of a hash database object.
   `hdb' specifies the hash database object.
   `kstr' specifies the string of the key.
   If successful, the return value is true, else, it is false.  False is returned if there is
   no record corresponding the condition. */
bool tchdbiterinit3(TCHDB *hdb, const char *kstr);

__TCHDB_CLINKAGEEND
#endif                                   /* duplication check */


/* END OF FILE */
