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

int main(int argc, char** argv)
{
    size_t mmap_len;
    int is_pmem;
    void* addr = pmem_map_file("/home/pmem0/test", (size_t)1024 * 1024 * 1024, PMEM_FILE_CREATE, 0777, &mmap_len, &is_pmem);
    printf("[addr:0x%x][mmap_len:%zu][is_pmem:%d]\n", (uint64_t)addr, mmap_len, is_pmem);
    return 0;
}