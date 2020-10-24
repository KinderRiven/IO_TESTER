#include "timer.h"
#include <assert.h>
#include <fcntl.h>
#include <linux/aio_abi.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#define DO_RW (1)
#define DO_SW (2)
#define DO_RR (3)
#define DO_SR (4)

struct thread_options {
    int type;
    int thread_id;
    size_t block_size;
    size_t total_size;
    double iops;
    char path[128];
};

void do_randwrite(int fd, size_t block_size, size_t total_size)
{
    uint64_t pos = 0;
    size_t count = total_size / block_size;
    void* buff;
    posix_memalign(&buff, block_size, block_size);
    memset(buff, 0xff, block_size);
    for (int i = 0; i < count; i++) {
        pwrite(fd, buff, block_size, pos);
        pos += block_size;
        // write(fd, buff, block_size);
    }
    free(buff);
}

void do_seqwrite(int fd, size_t block_size, size_t total_size)
{
    size_t count = total_size / block_size;
    size_t skip = (16 * 1024 + block_size);
    uint64_t pos = 0;
    void* buff;
    posix_memalign(&buff, block_size, block_size);
    memset(buff, 0xff, block_size);
    for (int i = 0; i < count; i++) {
        pwrite(fd, buff, block_size, pos);
        pos += skip;
        if (pos > total_size) {
            pos = 0;
        }
    }
    free(buff);
}

void do_randread(int fd, size_t block_size, size_t total_size)
{
    uint64_t pos = 0;
    size_t count = total_size / block_size;
    void* buff;
    posix_memalign(&buff, block_size, block_size);
    memset(buff, 0xff, block_size);
    for (int i = 0; i < count; i++) {
        pread(fd, buff, block_size, pos);
        pos += block_size;
    }
    free(buff);
}

void do_seqread(int fd, size_t block_size, size_t total_size)
{
    size_t count = total_size / block_size;
    size_t skip = (16 * 1024 + block_size);
    uint64_t pos = 0;
    void* buff;
    posix_memalign(&buff, block_size, block_size);
    memset(buff, 0xff, block_size);
    for (int i = 0; i < count; i++) {
        pread(fd, buff, block_size, pos);
        pos += skip;
        if (pos > total_size) {
            pos = 0;
        }
    }
    free(buff);
}

void* run_benchmark(void* options)
{
    struct thread_options* opt = (struct thread_options*)options;
    int fd;
    char file_name[32];
    sprintf(file_name, "%s/%d.io", opt->path, opt->thread_id);

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(opt->thread_id, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
        printf("threadpool, set thread affinity failed.\n");
    }
    fd = open(file_name, O_RDWR | O_DIRECT, 0777);

    Timer timer;
    timer.Start();
    switch (opt->type) {
    case DO_RW:
        do_randwrite(fd, opt->block_size, opt->total_size);
        break;
    case DO_SW:
        do_seqwrite(fd, opt->block_size, opt->total_size);
        break;
    case DO_RR:
        do_randread(fd, opt->block_size, opt->total_size);
        break;
    case DO_SR:
        do_seqread(fd, opt->block_size, opt->total_size);
        break;
    default:
        printf("error test type!\n");
        break;
    }
    timer.Stop();
    double seconds = timer.GetSeconds();
    double latency = 1000000000.0 * seconds / (opt->total_size / opt->block_size);
    double iops = 1000000000.0 / latency;
    printf("[%d][TIME:%.2f][IOPS:%.2f]\n", opt->thread_id, seconds, iops);
    opt->iops = iops;
    close(fd);
    return nullptr;
}

// #define USE_FALLOCATE
int main(int argc, char** argv)
{
    if (argc < 6) {
        printf("./posix [rw] [io_path] [num_thread] [block_size(B)] [total_size(MB)]\n");
        printf("for example: ./posix 2 /home/hanshukai/p3700_dir1 4 4096 2048\n");
        exit(1);
    }

    pthread_t thread_id[32];
    struct thread_options options[32];
    int type = atol(argv[1]);
    // argv[2] is test path
    int num_thread = atol(argv[3]);
    size_t block_size = atol(argv[4]); // B
    size_t total_size = atol(argv[5]); // MB
    total_size *= (1024 * 1024);

    for (int i = 0; i < num_thread; i++) {
        int fd;
        char file_name[32];
        sprintf(file_name, "%s/%d.io", argv[2], i);
        fd = open(file_name, O_RDWR | O_CREAT, 0777);
        fallocate(fd, 0, 0, total_size);
        close(fd);
    }

    for (int i = 0; i < num_thread; i++) {
        options[i].type = type;
        strcpy(options[i].path, argv[2]);
        options[i].thread_id = i;
        options[i].block_size = block_size;
        options[i].total_size = total_size;
        printf("[%02d] pthread create new thread.\n", i);
        pthread_create(thread_id + i, nullptr, run_benchmark, (void*)&options[i]);
    }

    for (int i = 0; i < num_thread; i++) {
        pthread_join(thread_id[i], nullptr);
    }

    double sum_iops = 0;
    for (int i = 0; i < num_thread; i++) {
        sum_iops += options[i].iops;
    }
    printf("[SUM][TYPE:%d][IOPS:%.2f][BW:%.2fMB/s]\n", type, sum_iops, sum_iops * block_size / (1024 * 1024));
    return 0;
}