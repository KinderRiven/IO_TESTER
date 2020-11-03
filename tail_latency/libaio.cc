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
public:
    int read_type;
    int write_type;
    double read_percentage;

public:
    int thread_id;
    char path[128];
    uint64_t time;
    size_t file_size;
    size_t block_size;
};

struct worker_options {
public:
    int read_fd;
    int write_fd;
    int thread_id;
    size_t file_size;
    size_t block_size;
    double read_percentage;

public:
    uint64_t run_time;
    std::vector<uint64_t> vec_latency;
};

static int io_depth = 8;

char g_result_save_path[128];

// async (libaio)
static int io_destroy(aio_context_t ctx)
{
    return syscall(__NR_io_destroy, ctx);
}

static int io_setup(unsigned nr, aio_context_t* ctxp)
{
    return syscall(__NR_io_setup, nr, ctxp);
}

static int io_submit(aio_context_t ctx, long nr, struct iocb** iocbpp)
{
    return syscall(__NR_io_submit, ctx, nr, iocbpp);
}

static int io_getevents(aio_context_t ctx, long min_nr, long max_nr,
    struct io_event* events, struct timespec* timeout)
{
    return syscall(__NR_io_getevents, ctx, min_nr, max_nr, events, timeout);
}

void do_libaio(struct worker_options* options)
{
    int ret;
    void* vbuff;
    size_t queue_size = io_depth;
    size_t current_count = 0;
    posix_memalign(&vbuff, block_size, block_size * queue_size);
    char* buff = (char*)vbuff;
    memset(buff, 0xff, block_size * queue_size);
    size_t count = total_size / (block_size * queue_size);
    aio_context_t ioctx;
    struct iocb iocb[128];
    struct io_event events[128];
    struct iocb* iocbs[128];

    ioctx = 0;
    io_setup(128, &ioctx);
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < queue_size; j++) {
            iocb[j].aio_fildes = fd;
            iocb[j].aio_nbytes = block_size;
            iocb[j].aio_offset = block_size * current_count;
            iocb[j].aio_lio_opcode = IOCB_CMD_PWRITE;
            iocb[j].aio_buf = (uint64_t)&buff[j * block_size];
            iocbs[j] = &iocb[j];
            current_count++;
        }
        ret = io_submit(ioctx, queue_size, iocbs);
        assert(ret == queue_size);
        ret = io_getevents(ioctx, ret, ret, events, nullptr);
        assert(ret == queue_size);
    }
    io_destroy(ioctx);
    free(vbuff);
}

void* run_benchmark(void* options)
{
    struct thread_options* _opt = (struct thread_options*)options;
    struct worker_options _wopt;

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(_opt->thread_id, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
        printf("threadpool, set thread affinity failed.\n");
    }

    char _file_name[32];
    sprintf(_file_name, "%s/%d.io", _opt->path, _opt->thread_id);
    _wopt.fd = open(_file_name, O_RDWR | O_DIRECT, 0777);
    _wopt.thread_id = _opt->thread_id;
    _wopt.block_size = _opt->block_size;
    _wopt.file_size = _opt->file_size;
    _wopt.run_time = _opt->time;

    switch (_opt->type) {
    case (OPT_WRITE | OPT_RANDOM):
        do_randwrite(&_wopt);
        break;
    case (OPT_WRITE):
        do_seqwrite(&_wopt);
        break;
    case (OPT_READ | OPT_RANDOM):
        do_randread(&_wopt);
        break;
    case (OPT_READ):
        do_seqread(&_wopt);
        break;
    default:
        printf("ERROR TYPE!\n");
        break;
    }
    close(_wopt.fd);
    return nullptr;
}

// #define USE_FALLOCATE
int main(int argc, char** argv)
{
    if (argc < 9) {
        printf("./libaio [path] [time] [num_thread] [read_percentage] [write_type] [write_block_size] [read_type] [read_block_size]\n");
        exit(1);
    }

    time_t _t = time(NULL);
    struct tm* _lt = localtime(&_t);
    sprintf(g_result_save_path, "%04d%02d%02d_%02d%02d%02d_aio", _lt->tm_year, _lt->tm_mon, _lt->tm_mday, _lt->tm_hour, _lt->tm_min, _lt->tm_sec);
    mkdir(g_result_save_path, 0777);

    pthread_t thread_id[32];
    struct thread_options options[32];

    char* _path = argv[1];
    uint64_t _time = atol(argv[2]);
    size_t _file_size = (size_t)FILE_SIZE * (1024 * 1024 * 1024);

    int _num_thread = atol(argv[3]);
    double _read_percentage = atof(argv[4]);

    int _write_type = atol(argv[5]);
    size_t _write_block_size = atol(argv[6]);

    int _read_type = atol(argv[7]);
    size_t _read_block_size = atol(argv[8]);

    /*
    for (int i = 0; i < _num_thread; i++) {
        int __fd;
        char __file_name[32];
        // 1.create read file
        sprintf(__file_name, "%s/%d.io", _path, i);
        __fd = open(__file_name, O_RDWR | O_CREAT, 0777);
        fallocate(__fd, 0, 0, _file_size);
        close(__fd);
    }
    */

    for (int i = 0; i < _num_thread; i++) {
        options[i].read_type = OPT_WRITE;
        options[i].read_type |= _write_type;
        options[i].write_type = OPT_READ;
        options[i].write_type |= _read_type;
        strcpy(options[i].path, _path);
        options[i].time = _time * (1000000000); //ns
        options[i].thread_id = i;
        options[i].block_size = _write_block_size;
        options[i].file_size = _file_size;
        printf("[%02d] pthread create new thread.\n", i);
        pthread_create(thread_id + options[i].thread_id, nullptr, run_benchmark, (void*)&options[i]);
    }

    for (int i = 0; i < _num_thread; i++) {
        pthread_join(thread_id[i], nullptr);
    }
    return 0;
}