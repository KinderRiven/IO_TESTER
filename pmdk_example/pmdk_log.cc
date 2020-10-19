#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "pmdk/libpmem.h"
#include "pmdk/libpmemlog.h"

int main(int argc, char** argv)
{
    // void* addr = pmem_map_file("/home/pmem0/test", (size_t)1024 * 1024 * 1024, PMEM_FILE_CREATE, 0777, &mmap_len, &is_pmem);
    PMEMlogpool* _pool = pmemlog_create("/home/pmem0/pmdk_pool", (size_t)2 * 1024 * 1024 * 1024, 0666);
    size_t _nb = pmemlog_nbyte(_pool);
    printf("%zuMB\n", _nb / (1024 * 1024));
    return 0;
}