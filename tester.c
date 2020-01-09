#include "timer.h"
#include <fcntl.h>
#include <libaio.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <unistd.h>

struct thread_options {
    int type;
    int thread_id;
    size_t block_size;
    size_t total_size;
    double iops;
};

#define IO_READ_WRITE (1)
#define IO_DIRECT_ACCESS (2)
#define IO_MMAP (3)
#define IO_LIBAIO (4)

// read/write
void io_read_write(int fd, size_t block_size, size_t total_size)
{
    size_t count = total_size / block_size;
    void* buff;
    posix_memalign(&buff, block_size, block_size);
    memset(buff, 0xff, block_size);

    for (int i = 0; i < count; i++) {
        write(fd, buff, block_size);
        fsync(fd);
    }
    free(buff);
}

// direct_io
void io_direct_access(int fd, size_t block_size, size_t total_size)
{
    size_t count = total_size / block_size;
    void* buff;
    posix_memalign(&buff, block_size, block_size);
    memset(buff, 0xff, block_size);

    for (int i = 0; i < count; i++) {
        write(fd, buff, block_size);
    }
    free(buff);
}

// mmap
void io_mmap(int fd, size_t block_size, size_t total_size)
{
    void* dest = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, fd, 0);
    size_t count = total_size / block_size;
    void* buff;
    posix_memalign(&buff, block_size, block_size);
    memset(buff, 0xff, block_size);
    char* ptr = (char*)dest;

    for (int i = 0; i < count; i++) {
        memcpy(ptr, buff, block_size);
        msync(ptr, block_size, MS_SYNC);
        ptr += block_size;
    }

    free(buff);
    munmap(dest, total_size);
}

// async (libaio)
void io_libaio(int fd, size_t block_size, size_t total_size)
{
    int ret;
    void* vbuff;
    size_t queue_size = 16;
    size_t current_count = 0;
    posix_memalign(&vbuff, block_size, block_size * queue_size);

    char* buff = (char*)vbuff;
    memset(buff, 0xff, block_size * queue_size);
    size_t count = total_size / (block_size * queue_size);

    io_context_t ioctx;
    struct iocb iocb[128];
    struct io_event events[128];
    struct iocb *iocbs[128];

    io_setup(128, &ioctx);

    for (int i = 0; i < count; i++) {
        for (int j = 0; j < queue_size; j++) {
            io_prep_pwrite(&iocb[j], fd, &buff[j * block_size], block_size, block_size * current_count);
            iocbs[j] = &iocb[j];
            current_count++;
        }
        ret = io_submit(ioctx, queue_size, iocbs);
        ret = io_getevents(ioctx, ret, ret, events, NULL);
    }
    free(vbuff);
}

void* run_benchmark(void* options)
{
    struct thread_options* opt = (struct thread_options*)options;
    int fd;
    char file_name[32];
    sprintf(file_name, "%d.io", opt->thread_id);

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(opt->thread_id, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
        printf("threadpool, set thread affinity failed.\n");
    }

    if (opt->type == IO_READ_WRITE) {
        fd = open(file_name, O_RDWR | O_CREAT, 0777);
    } else if (opt->type == IO_DIRECT_ACCESS) {
        fd = open(file_name, O_RDWR | O_DIRECT | O_CREAT, 0777);
    } else if (opt->type == IO_MMAP) {
        fd = open(file_name, O_RDWR | O_CREAT, 0777);
    } else if (opt->type == IO_LIBAIO) {
        fd = open(file_name, O_RDWR | O_DIRECT | O_CREAT, 0777);
    }

    Timer timer;
    timer.Start();

    switch (opt->type) {
    case IO_READ_WRITE:
        io_read_write(fd, opt->block_size, opt->total_size);
        break;
    case IO_DIRECT_ACCESS:
        io_direct_access(fd, opt->block_size, opt->total_size);
        break;
    case IO_MMAP:
        io_mmap(fd, opt->block_size, opt->total_size);
        break;
    case IO_LIBAIO:
        io_libaio(fd, opt->block_size, opt->total_size);
        break;
    }

    timer.Stop();
    double seconds = timer.GetSeconds();
    double latency = 1000000000.0 * seconds / (opt->total_size / opt->block_size);
    double iops = 1000000000.0 / latency;
    printf("[%d][TIME:%.2f][IOPS:%.2f]\n", opt->thread_id, seconds, iops);
    opt->iops = iops;
    close(fd);
    return NULL;
}

// #define USE_FALLOCATE

int main(int argc, char** argv)
{
    pthread_t thread_id[32];
    struct thread_options options[32];
    int type = atol(argv[1]);
    int num_thread = atol(argv[2]);
    size_t block_size = atol(argv[3]); // B
    size_t total_size = atol(argv[4]); // MB
    total_size *= (1024 * 1024);

#ifdef USE_FALLOCATE
    for (int i = 0; i < num_thread; i++) {
        int fd;
        char file_name[32];
        sprintf(file_name, "%d.io", i);
        fd = open(file_name, O_RDWR | O_CREAT, 0777);
        fallocate(fd, 0, 0, total_size);
        close(fd);
    }
#endif

    for (int i = 0; i < num_thread; i++) {
        options[i].type = type;
        options[i].thread_id = i;
        options[i].block_size = block_size;
        options[i].total_size = total_size;
        pthread_create(thread_id + i, NULL, run_benchmark, (void*)&options[i]);
    }

    for (int i = 0; i < num_thread; i++) {
        pthread_join(thread_id[i], NULL);
    }

    double sum_iops = 0;
    for (int i = 0; i < num_thread; i++) {
        sum_iops += options[i].iops;
    }
    printf("[SUM][TYPE:%d][IOPS:%.2f][BW:%.2fMB/s]\n", type, sum_iops, sum_iops * block_size / (1024 * 1024));
    return 0;
}