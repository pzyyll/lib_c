#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([snslib], [1.0], [michaelzhao@tencent.com])
AM_INIT_AUTOMAKE([snslib], [1.0], [michaelzhao@tencent.com])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

CXXFLAGS="-Wall -g -D_GNU_SOURCE -DLINUX -fPIC"
CFLAGS="-Wall -g -D_GNU_SOURCE -DLINUX -fPIC"

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lbz2':
#AC_CHECK_LIB([bz2], [main])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [pthread_create])
# FIXME: Replace `main' with a function in `-lz':
#AC_CHECK_LIB([z], [main])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h inttypes.h limits.h locale.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/param.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([atexit ftruncate gettimeofday inet_ntoa localtime_r memmove memset mkdir modf munmap pow realpath regcomp rmdir socket sqrt strchr strerror strrchr strstr strtoull tzset])

AC_CONFIG_FILES([Makefile
				 comm/Makefile
				 comm/hdb/Makefile
				 comm/bus_api/Makefile
				 comm/file_lock/Makefile
				 comm/hash_db/Makefile
				 comm/ini_file/Makefile
				 comm/log/Makefile
				 comm/sem_lock/Makefile
				 comm/share_mem/Makefile
				 comm/shm_queue/Makefile
				 comm/shm_queue_ex/Makefile
				 comm/util/Makefile
				 comm/pet_mysql/Makefile
				 comm/process_manager/Makefile
				 comm/tcp_client/Makefile
				 comm/safe_tcp_client/Makefile
				 comm/udp_client/Makefile
				 comm/compress/Makefile
				 comm/uuid/Makefile
				 api/Makefile
				 api/adopt_api/Makefile
				 api/route_info_api/Makefile
				 api/transaction_api/Makefile
				 api/oidb_api/Makefile
				 api/proto/Makefile
				 api/web_api/Makefile
				 api/paysvr_api/Makefile
				 api/msg_centre_util/Makefile
				 api/shmconf_api/Makefile
				 api/vip_api/Makefile
				 api/vip_proxy_api/Makefile
				 api/sns_oidb_api/Makefile
				 api/tt_feeds_api/Makefile
				 api/qqgamefeeds_api/Makefile
                 ])
AC_OUTPUT
