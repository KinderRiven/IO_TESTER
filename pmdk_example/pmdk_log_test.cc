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
    int _is_pmem;
    size_t _mmap_len;
    PMEMlogpool* _pool;
    char _path[] = "/home/pmem0/pool";

    // void* addr = pmem_map_file(_path, (size_t)2 * 1024 * 1024 * 1024, PMEM_FILE_CREATE, 0666, &_mmap_len, &_is_pmem);
    _pool = pmemlog_create(_path, (size_t)2 * 1024 * 1024 * 1024, 0666);

    if (_pool == nullptr) {
        _pool = pmemlog_open(_path);
    }

    if (_pool == nullptr) {
        perror(_path);
        exit(1);
    }

    size_t _nb = pmemlog_nbyte(_pool);
    printf("%zuMB\n", _nb / (1024 * 1024));

    char _buff[128] = "hello, world!";
    int _res = pmemlog_append(_pool, _buff, strlen(_buff));

    printf("%d\n", _res);
    return 0;
}