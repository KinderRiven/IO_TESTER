#include "timer.h"
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct thread_options {
public:
    int type;
    int thread_id;
    size_t block_size;
    size_t total_size;

public:
    double iops;
};

#define IO_READ_WRITE (1)
#define IO_DIRECT_ACCESS (2)
#define IO_MMAP (3)
#define IO_LIBAIO (4)

// read/write
void io_read_write(int thread_id, size_t block_size, size_t total_size)
{
    char file_name[32];
    sprintf(file_name, "%d.io", thread_id);
    int fd = open(file_name, O_RDWR | O_CREAT, 0777);
    size_t count = total_size / block_size;
    char* buff = (char*)malloc(block_size);
    memset(buff, 0xff, block_size);

    for (int i = 0; i < count; i++) {
        write(fd, buff, block_size);
    }

    free(buff);
    close(fd);
}

// direct_io
void io_direct_access(int thread_id, size_t block_size, size_t total_size)
{
}

// mmap
void io_mmap(int thread_id, size_t block_size, size_t total_size)
{
}

// async (libaio)
void io_libaio(int thread_id, size_t block_size, size_t total_size)
{
}

void* run_benchmark(void* options)
{
    struct thread_options* opt = (struct thread_options*)options;
    Timer timer;
    timer.Start();

    switch (opt->type) {
    case IO_READ_WRITE:
        io_read_write(opt->thread_id, opt->block_size, opt->total_size);
        break;
    case IO_DIRECT_ACCESS:
        io_direct_access(opt->thread_id, opt->block_size, opt->total_size);
        break;
    case IO_MMAP:
        io_mmap(opt->thread_id, opt->block_size, opt->total_size);
        break;
    case IO_LIBAIO:
        io_libaio(opt->thread_id, opt->block_size, opt->total_size);
        break;
    }

    timer.Stop();
    double seconds = timer.GetSeconds();
    double latency = 1000000000.0 * seconds / (opt->total_size / opt->block_size);
    double iops = 1000000000.0 / latency;
    printf("[%d][TIME:%.2f][IOPS:%.2f]\n", opt->thread_id, seconds, iops);
    opt->iops = iops;
    return NULL;
}

int main(int argc, char** argv)
{
    pthread_t thread_id[32];
    struct thread_options options[32];
    int type = atol(argv[1]);
    int num_thread = atol(argv[2]);
    size_t block_size = atol(argv[3]); // B
    size_t total_size = atol(argv[4]); // MB

    for (int i = 0; i < num_thread; i++) {
        options[i].type = type;
        options[i].thread_id = i;
        options[i].block_size = block_size;
        options[i].total_size = total_size * (1024 * 1024);
        pthread_create(thread_id + i, NULL, run_benchmark, (void*)&options[i]);
    }

    for (int i = 0; i < num_thread; i++) {
        pthread_join(thread_id[i], NULL);
    }

    double sum_iops = 0;
    for (int i = 0; i < num_thread; i++) {
        sum_iops += options[i].iops;
    }
    printf("[SUM][IOPS:%.2f][BW:%.2fMB/s]\n", sum_iops, sum_iops * block_size / (1024 * 1024));
    return 0;
}