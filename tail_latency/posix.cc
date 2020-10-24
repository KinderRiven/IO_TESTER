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
#include <vector>

#define OPT_RANDOM (1)
#define OPT_READ (0)
#define OPT_WRITE (2)

// GB
#define FILE_SIZE (16)
#define RANDOM_SKIP (16384)

struct thread_options {
public:
    int type;
    int thread_id;
    char path[128];
    size_t file_size;
    size_t block_size;
};

struct worker_options {
public:
    int fd;
    size_t file_size;
    size_t block_size;

public:
    uint64_t run_time;
    std::vector<uint64_t> vec_latency;
};

void do_randwrite(struct worker_options* options)
{
    Timer _timer;
    Timer _latency;
    uint64_t _pos = 0;
    int _fd = options->fd;
    size_t _fs = options->file_size;
    size_t _bs = options->block_size;
    uint64_t _run_time = options->run_time;

    void* _buff;
    posix_memalign(&_buff, _bs, _bs);
    memset(_buff, 0xff, _bs);

    _timer.Start();
    while (true) {
        _latency.Start();
        pwrite(_fd, _buff, _bs, _pos);
        _latency.Stop();

        options->vec_latency.push_back(_latency.Get());
        _timer.Stop();
        if (_timer.Get() > _run_time) {
            break;
        }

        _pos += RANDOM_SKIP;
        if (_pos > _fs) {
            _pos = 0;
        }
    }
    free(_buff);
}

void do_seqwrite(struct worker_options* options)
{
    Timer _timer;
    Timer _latency;
    uint64_t _pos = 0;
    int _fd = options->fd;
    size_t _fs = options->file_size;
    size_t _bs = options->block_size;
    uint64_t _run_time = options->run_time;

    void* _buff;
    posix_memalign(&_buff, _bs, _bs);
    memset(_buff, 0xff, _bs);

    _timer.Start();
    while (true) {
        _latency.Start();
        pwrite(_fd, _buff, _bs, _pos);
        _latency.Stop();

        options->vec_latency.push_back(_latency.Get());
        _timer.Stop();
        if (_timer.Get() > _run_time) {
            break;
        }

        _pos += _bs;
        if (_pos > _fs) {
            _pos = 0;
        }
    }
    free(_buff);
}

void do_randread(struct worker_options* options)
{
}

void do_seqread(struct worker_options* options)
{
}

void* run_benchmark(void* options)
{
    struct thread_options* _opt = (struct thread_options*)options;
    struct worker_options _w_opt;

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(_opt->thread_id, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
        printf("threadpool, set thread affinity failed.\n");
    }

    char _file_name[32];
    sprintf(_file_name, "%s/%d.io", _opt->path, _opt->thread_id);
    _wopt.fd = open(_file_name, O_RDWR | O_DIRECT, 0777);
    _wopt.block_size = _opt->block_size;
    _wopt.file_size = _opt->file_size;

    switch (opt->type) {
    case DO_RW:
        do_randwrite(&_wopt);
        break;
    case DO_SW:
        do_seqwrite(&_wopt);
        break;
    case DO_RR:
        do_randread(&_wopt);
        break;
    case DO_SR:
        do_seqread(&_wopt);
        break;
    default:
        printf("ERROR TYPE!\n");
        break;
    }
    close(_wopt->fd);
    return nullptr;
}

// #define USE_FALLOCATE
int main(int argc, char** argv)
{
    if (argc < 9) {
        printf("./posix [path] [time] [write_type] [num_write_thread] [write_block_size] [read_type] [num_read_thread] [read_block_size]\n");
        exit(1);
    }

    pthread_t thread_id[32];
    struct thread_options options[32];

    char* _path = argv[1];
    uint64_t _time = atol(argv[2]);
    size_t _file_size = (size_t)FILE_SIZE * (1024 * 1024 * 1024);

    int _write_type = atol(argv[3]);
    int _num_write_thread = atol(argv[4]);
    size_t _write_block_size = atol(argv[5]);

    int _read_type = atol(argv[6]);
    int _num_read_thread = atol(argv[7]);
    size_t _read_block_size = atol(argv[8]);

    int _num_thread = _num_write_thread + _num_read_thread;

    for (int i = 0; i < num_thread; i++) {
        int __fd;
        char __file_name[32];
        sprintf(__file_name, "%s/%d.io", _path, i);
        __fd = open(__file_name, O_RDWR | O_CREAT, 0777);
        fallocate(__fd, 0, 0, _file_size);
        close(__fd);
    }

    for (int i = 0; i < _num_thread; i++) {
        if (i < _num_write_thread) {
            options[i].type = OPT_WRITE;
            if (_write_type) {
                options[i].type |= OPT_RANDOM;
            }
        } else {
            options[i].type = OPT_READ;
            if (_write_type) {
                options[i].type |= OPT_RANDOM;
            }
        }
        strcpy(options[i].path, argv[2]);
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