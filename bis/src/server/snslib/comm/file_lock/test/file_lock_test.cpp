#include <stdio.h>
#include <stdlib.h>
#include "comm/file_lock/file_lock.h"

using namespace snslib;

int main(int argc, char* argv[])
{
	int iRetVal = 0;
	CFileLock theLock;

	iRetVal = theLock.Init("test1");
	if (iRetVal != CFileLock::SUCCESS)
	{
		printf("FileLock Init failed, ret=%d\n", iRetVal);
		return -1;
	}

	iRetVal = theLock.Lock(CFileLock::FILE_LOCK_WRITE, 100, 100);
	if (iRetVal != CFileLock::SUCCESS)
	{
		printf("FileLock Lock failed, ret=%d\n", iRetVal);
		return -1;
	}

	printf("Locked area 100 size 100 for read\n");
	fgetc(stdin);

    iRetVal = theLock.Lock(CFileLock::FILE_LOCK_WRITE, 100, 100);
    if (iRetVal != CFileLock::SUCCESS)
    {
        printf("FileLock Lock failed, ret=%d\n", iRetVal);
        return -1;
    }

    printf("Locked area 100 size 100 for read\n");
    fgetc(stdin);

	return 0;
}


