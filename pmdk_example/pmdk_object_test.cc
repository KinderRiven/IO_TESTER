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
#include "pmdk/libpmemobj.h"

int main(int argc, char** argv)
{
    int _is_pmem;
    size_t _mmap_len;
    size_t _pool_size = 2UL * 1024 * 1024 * 1024;
    PMEMlogpool* _pool = nullptr;
    const char _path[] = "/home/pmem0/pool";

    void* addr = pmem_map_file(_path, (size_t)2 * 1024 * 1024 * 1024, PMEM_FILE_CREATE, 0666, &_mmap_len, &_is_pmem);
    PMEMobjpool* _pool = pmemobj_create(_path, "btree", _pool_size, 0666);

    if (_pool == nullptr) {
        printf("create error!\n");
    }

    return 0;
}