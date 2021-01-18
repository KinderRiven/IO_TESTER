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

static PMEMobjpool* create_one_pool(const char* path, const char* layout, size_t psize)
{
    PMEMobjpool* _pool = nullptr;
    _pool = pmemobj_create(path, layout, psize);

    if (_pool == nullptr) {
        printf("%s-%s existed, now just open!\n", path, layout);
        _pool = pmemobj_open(path, layout);
    }
    return _pool;
}

int main(int argc, char** argv)
{
    size_t _pool_size = 2UL * 1024 * 1024 * 1024;
    char _path[128] = "/home/pmem0/pool";
    char _layout1[128] = "index";
    char _layout2[128] = "data";

    PMEMobjpool* _p1 = create_one_pool(_path, _layout1, _pool_size);
    if (_p1 == nullptr) {
        printf("p1 is nullptr!\n");
    }

    PMEMobjpool* _p2 = create_one_pool(_path, _layout2, _pool_size);
    if (_p2 == nullptr) {
        printf("p2 is nullptr!\n");
    }
    return 0;
}