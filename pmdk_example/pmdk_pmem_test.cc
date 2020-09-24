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

void do_write(void* base, size_t space_size, size_t data_size, size_t block_size)
{
    char* buff = new char[block_size];
    size_t one_turn = space_size / block_size;
    size_t turn_count = data_size / space_size;

    printf("[turn_count:%d][one_turn:%d][bs:%zu]\n", turn_count, one_turn, block_size);

    for (size_t i = 0; i < turn_count; i++) {
        char* addr = (char*)base;
        for (size_t j = 0; j < one_turn; j++) {
            pmem_memcpy_persist((void*)(addr), buff, block_size);
            addr += block_size;
        }
    }
}

void do_read(void* base, size_t space_size, size_t data_size, size_t block_size)
{
}

int main(int argc, char** argv)
{
    size_t mmap_len;
    size_t bs = atol(argv[1]);
    int is_pmem;
    void* addr = pmem_map_file("/home/pmem0/test", (size_t)1024 * 1024 * 1024, PMEM_FILE_CREATE, 0777, &mmap_len, &is_pmem);
    printf("[addr:0x%x][mmap_len:%zu][is_pmem:%d]\n", (uint64_t)addr, mmap_len, is_pmem);
    do_write(addr, mmap_len, mmap_len * 5, bs);
    return 0;
}