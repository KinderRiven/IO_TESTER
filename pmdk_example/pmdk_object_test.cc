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
    PMEMobjpool* _index = nullptr;
    PMEMobjpool* _data = nullptr;
    const char _path[] = "/home/pmem0/pool";

    // void* addr = pmem_map_file(_path, (size_t)2 * 1024 * 1024 * 1024, PMEM_FILE_CREATE, 0666, &_mmap_len, &_is_pmem);
    _index = pmemobj_create(_path, "index", _pool_size, 0666);
    if (_index == nullptr) {
        printf("existed index!\n");
        _index = pmemobj_open(_path, "index");
    }
    if (_index == nullptr) {
        printf("errer create for index!\n");
        exit(1);
    }

    _data = pmemobj_create(_path, "data", _pool_size, 0666);
    if (_data == nullptr) {
        printf("existed data!\n");
        _data = pmemobj_open(_path, "data");
    }
    if (_data == nullptr) {
        printf("errer create for data!\n");
        exit(1);
    }
    return 0;
}