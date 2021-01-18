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

struct node_t {
public:
    uint64_t value;
};

struct proot_t {
public:
    PMEMoid nodes[128];
};

static void create_pmem_file(const char* path, size_t psize)
{
    int _is_pmem;
    size_t _mmap_len;
    void *_addr = pmem_map_file(path, psize, PMEM_FILE_CREATE, 0666, &_mmap_len, &_is_pmem);
    printf("[addr:0x%x][mmap_len:%zu][is_pmem:%d]\n", (uint64_t)_addr, _mmap_len, _is_pmem);
}

static PMEMobjpool* create_one_pool(const char* path, const char* layout, size_t psize)
{
    PMEMobjpool* _pool = nullptr;
    _pool = pmemobj_create(path, layout, psize, 0666);

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

    create_pmem_file(_path, _pool_size * 10);

    PMEMobjpool* _p1 = create_one_pool(_path, _layout1, _pool_size);
    if (_p1 == nullptr) {
        printf("p1 is nullptr!\n");
    }

    // PMEMobjpool* _p2 = create_one_pool(_path, _layout2, _pool_size);
    // if (_p2 == nullptr) {
    //     printf("p2 is nullptr!\n");
    // }

    PMEMoid _proot = pmemobj_root(_p1, sizeof(proot_t));
    proot_t* _mroot = (proot_t*)pmemobj_direct(_proot);

    for (int j = 0; j < 1000000; j++) {
        for (int i = 0; i < 4; i++) {
            node_t* __node = (node_t*)pmemobj_direct(_mroot->nodes[i]);
            if (__node == nullptr) {
                pmemobj_alloc(_p1, &_mroot->nodes[i], sizeof(node_t), 1, nullptr, nullptr);
                __node = (node_t*)pmemobj_direct(_mroot->nodes[i]);
                __node->value = 0;
            }
            printf("%d:%llu[%llu/%llu]\n", i, __node->value, _mroot->nodes[i].pool_uuid_lo, _mroot->nodes[i].off);
            __node->value += i;
            pmemobj_persist(_p1, __node, sizeof(node_t));
        }
    }
    pmemobj_close(_p1);
    return 0;
}