#include "timer.h"
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "spdk/env.h"
#include "spdk/nvme.h"
#include "spdk/stdinc.h"
#include "spdk/vmd.h"

struct spdk_device_t {
public:
    std::string name;
    std::string transport_string;
    struct spdk_nvme_transport_id trid;

public:
    struct spdk_nvme_ctrlr* ctrlr;
    const struct spdk_nvme_ctrlr_opts* opts;
    const struct spdk_nvme_ctrlr_data* cdata;

public:
    uint32_t num_ns;
    uint64_t ns_capacity;
    struct spdk_nvme_ns* ns;
};

static bool probe_cb(void* cb_ctx, const struct spdk_nvme_transport_id* trid, struct spdk_nvme_ctrlr_opts* opts)
{
    printf("prob_cb\n");
}

static void attach_cb(void* cb_ctx, const struct spdk_nvme_transport_id* trid, struct spdk_nvme_ctrlr* ctrlr, const struct spdk_nvme_ctrlr_opts* opts)
{
    printf("attach_cb\n");

    spdk_device_t* _device = (spdk_device_t*)cb_ctx;
    _device->ctrlr = ctrlr;
    _device->opts = opts;
    _device->cdata = spdk_nvme_ctrlr_get_data(ctrlr);
    _device->num_ns = spdk_nvme_ctrlr_get_num_ns(ctrlr);

    // We assume that one device only cconfigure one namespace
    assert(_device->num_ns == 1);

    _device->ns = spdk_nvme_ctrlr_get_ns(ctrlr, 1);
    _device->ns_capacity = spdk_nvme_ns_get_size(_device->ns);
}

static void remove_cb(void* cb_ctx, struct spdk_nvme_ctrlr* ctrlr)
{
    printf("remove_cb\n");
}

void init_spdk_device(spdk_device_t* device)
{
    int res;
    struct spdk_env_opts opts;

    spdk_env_opts_init(&opts);
    printf("spdk_env_opts_init\n");

    res = spdk_env_init(&opts);
    printf("spdk_env_init.[%d]\n", res);

    res = spdk_nvme_probe(device->trid, (void*)device, probe_cb, attach_cb, remove_cb);
    printf("spdk_nvme_probe.[%d]\n", res);
}

void write_cb(void* arg, const struct spdk_nvme_cpl* completion)
{
    spdk_free(arg);
}

/*
void do_seqwrite(spdk_device_t* device, size_t block_size, size_t total_size)
{
    assert(block_size % 512 == 0);
    uint64_t k = 0;
    uint64_t n_lba = block_size / 512;
    uint64_t count = (total_size / block_size) / io_depth;
    struct spdk_nvme_qpair* qpair = spdk_nvme_ctrlr_alloc_io_qpair(device->ctrlr, NULL, 0);
    assert(qpair != nullptr);

    for (uint64_t i = 0; i < count; i++) {
        int c = 0;
        for (int j = 0; j < io_depth; j++) {
            char* buff = (char*)spdk_zmalloc(block_size, block_size, nullptr, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_DMA);
            memset(buff, 0xff, block_size);
            int rc = spdk_nvme_ns_cmd_write(device->ns, qpair, buff, k, n_lba, write_cb, (void*)buff, 0);
            assert(rc == 0);
            k += n_lba;
        }
        while (true) {
            int num = spdk_nvme_qpair_process_completions(qpair, 0);
            c += num;
            if (c == io_depth) {
                break;
            }
        }
    }
    spdk_nvme_ctrlr_free_io_qpair(qpair);
}
*/

// #define USE_FALLOCATE
int main(int argc, char** argv)
{
    int _res;
    spdk_device_t _dev;
    _dev.transport_string = "trtype:PCIe traddr:0000:19:00.0";
    _res = spdk_nvme_transport_id_parse(&_dev.trid, _dev.transport_string.c_str());

    init_spdk_device(&_dev);
    printf("[%s-%zuGB]\n", _dev.transport_string.c_str(), _dev.capacity / (1024 * 1024 * 1024));

    spdk_nvme_detach(_dev.ctrlr);
    return 0;
}