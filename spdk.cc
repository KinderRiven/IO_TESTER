#include "timer.h"
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

#include "spdk/env.h"
#include "spdk/nvme.h"
#include "spdk/stdinc.h"
#include "spdk/vmd.h"

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
};

struct spdk_device_t {
    struct spdk_nvme_ctrlr* ctrlr; // control
    struct spdk_nvme_ns* ns; // each device only has one namespace
    size_t capacity;
    uint64_t base;
    size_t size; // device size
};

spdk_device_t using_device;

int io_depth = 8;

static bool probe_cb(void* cb_ctx, const struct spdk_nvme_transport_id* trid, struct spdk_nvme_ctrlr_opts* opts)
{
    printf("function1 (%s)!\n", trid->traddr);
}

static void attach_cb(void* cb_ctx, const struct spdk_nvme_transport_id* trid, struct spdk_nvme_ctrlr* ctrlr, const struct spdk_nvme_ctrlr_opts* opts)
{
    spdk_device_t* device = (spdk_device_t*)cb_ctx;
    device->ctrlr = ctrlr;
    // const struct spdk_nvme_ctrlr_data* cdata = spdk_nvme_ctrlr_get_data(ctrlr);
    // printf("[%d-%d][%s-%s-%s]\n", cdata->vid, cdata->ssvid, cdata->sn, cdata->mn, cdata->fr);
    uint32_t num_ns = spdk_nvme_ctrlr_get_num_ns(ctrlr);
    printf("num namespace:%d\n", num_ns);
    assert(num_ns == 1);
    device->ns = spdk_nvme_ctrlr_get_ns(ctrlr, 1);
    device->capacity = spdk_nvme_ns_get_size(device->ns);
}

static void remove_cb(void* cb_ctx, const struct spdk_nvme_transport_id* trid, struct spdk_nvme_ctrlr* ctrlr, const struct spdk_nvme_ctrlr_opts* opts)
{
}

void init_spdk_device()
{
    printf(">>init_spdk_device\n");

    int res;
    printf("  1.init_spdk_device\n");
    struct spdk_env_opts opts;

    spdk_env_opts_init(&opts);
    printf("  2.spdk_env_opts_init\n");

    res = spdk_env_init(&opts);
    printf("  3.spdk_env_init = %d\n", res);

    res = spdk_vmd_init();
    printf("  4.spdk_vmd_init = %d\n", res);

    res = spdk_nvme_probe(nullptr, (void*)&using_device, probe_cb, attach_cb, nullptr);
    printf("  5.new decice %zuGB\n", using_device.capacity / (1024 * 1024 * 1024));
}

void write_cb(void* arg, const struct spdk_nvme_cpl* completion)
{
    int* finished = (int*)arg;
    if (spdk_nvme_cpl_is_error(completion)) {
        printf("Something erro in write_callback!\n");
    }
    *finished = 1;
}

void do_seqwrite(spdk_device_t* device, size_t block_size, size_t total_size)
{
    uint64_t k = 0;
    uint64_t count = (total_size / block_size) / io_depth;
    struct spdk_nvme_qpair* qpair = spdk_nvme_ctrlr_alloc_io_qpair(device->ctrlr, NULL, 0);
    assert(qpair != nullptr);

    // new write buffer
    char* buff = (char*)spdk_nvme_ctrlr_alloc_cmb_io_buffer(device->ctrlr, block_size); // 4KB
    if (buff == nullptr) {
        buff = (char*)spdk_zmalloc(block_size, block_size, nullptr, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_DMA);
    }
    memset(buff, 0xff, block_size);
    assert(buff != nullptr);

    for (uint64_t i = 0; i < count; i++) {
        int finished[32] = { 0 };
        int c = 0;
        for (int j = 0; j < io_depth; j++) {
            int rc = spdk_nvme_ns_cmd_write(device->ns, qpair, buff, k, 1, write_cb, (void*)&finished[j], 0);
            k++;
        }
        while (true) {
            int num = spdk_nvme_qpair_process_completions(qpair, io_depth);
            c += num;
            if (c > io_depth) {
                break;
            }
        }
    }
    spdk_nvme_ctrlr_free_io_qpair(qpair);
}

void do_randwrite(spdk_device_t* device, size_t block_size, size_t total_size)
{
}

void do_seqread(spdk_device_t* device, size_t block_size, size_t total_size)
{
}

void do_randread(spdk_device_t* device, size_t block_size, size_t total_size)
{
}

void* run_benchmark(void* options)
{
    struct thread_options* opt = (struct thread_options*)options;

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(opt->thread_id, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
        printf("threadpool, set thread affinity failed.\n");
    }

    Timer timer;
    timer.Start();
    switch (opt->type) {
    case DO_RW:
        do_randwrite(&using_device, opt->block_size, opt->total_size);
        break;
    case DO_SW:
        do_seqwrite(&using_device, opt->block_size, opt->total_size);
        break;
    case DO_RR:
        do_randread(&using_device, opt->block_size, opt->total_size);
        break;
    case DO_SR:
        do_seqread(&using_device, opt->block_size, opt->total_size);
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
    return nullptr;
}

// #define USE_FALLOCATE
int main(int argc, char** argv)
{
    if (argc < 6) {
        printf("./spdk [rw] [num_thread] [io_depth] [block_size(B)] [total_size(MB)]\n");
        exit(1);
    }

    pthread_t thread_id[32];
    struct thread_options options[32];
    int type = atol(argv[1]);
    int num_thread = atol(argv[2]);
    io_depth = atol(argv[3]);
    size_t block_size = atol(argv[4]); // B
    size_t total_size = atol(argv[5]); // MB
    total_size *= (1024 * 1024);

    init_spdk_device();

    for (int i = 0; i < num_thread; i++) {
        options[i].type = type;
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
    printf("[SUM][[TYPE:%d]IO_DEPTH:%d][IOPS:%.2f][BW:%.2fMB/s]\n", type, sum_iops, io_depth, sum_iops * block_size / (1024 * 1024));
    return 0;
}