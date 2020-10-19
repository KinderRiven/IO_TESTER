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
    char _path[] = "/home/pmem0/pool";
    int _is_pmem;
    size_t _mmap_len;
    // void* addr = pmem_map_file(_path, (size_t)2 * 1024 * 1024 * 1024, PMEM_FILE_CREATE, 0666, &_mmap_len, &_is_pmem);
    PMEMlogpool* _pool = pmemlog_create(_path, (size_t)2 * 1024 * 1024 * 1024, 0666);
    if (_pool == nullptr) {
        _pool = pmemlog_open(_path);
    }
    assert(_pool != nullptr);
    size_t _nb = pmemlog_nbyte(_pool);
    printf("%zuMB\n", _nb / (1024 * 1024));
    return 0;
}