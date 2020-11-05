#include "timer.h"
#include <algorithm>
#include <assert.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <vector>

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

struct worker_options {
public:
    int io_depth;
    double read_ratio; // = read : write
    size_t size;
    size_t read_bs;
    size_t write_bs;
    uint64_t time_ns;
};

char g_save_path[128];

static void result_output(const char* name, std::vector<uint64_t>& data)
{
    std::ofstream fout(name);
    if (fout.is_open()) {
        for (int i = 0; i < data.size(); i++) {
            fout << data[i] << std::endl;
        }
        fout.close();
    }
}

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

    res = spdk_nvme_probe(&device->trid, (void*)device, probe_cb, attach_cb, remove_cb);
    printf("spdk_nvme_probe.[%d]\n", res);
}

struct cb_t {
public:
    void* buff;
    Timer timer;
};

void write_cb(void* arg, const struct spdk_nvme_cpl* completion)
{
    cb_t* _cb = (cb_t*)arg;
    _cb->timer.Stop();
    spdk_free(_cb->buff);
}

void read_cb(void* arg, const struct spdk_nvme_cpl* completion)
{
    cb_t* _cb = (cb_t*)arg;
    _cb->timer.Stop();
    spdk_free(_cb->buff);
}

static void get_avg_latency(std::vector<uint64_t>& vec_opt_latency)
{
    size_t size = vec_opt_latency.size();
    uint64_t sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += vec_opt_latency[i];
    }
    sum /= size;
    printf("  [Average]:%9lluns\n", sum);
    return;
}

static void get_tail_latency(std::vector<uint64_t>& vec_opt_latency, double p)
{
    size_t idx;
    size_t size;
    size = vec_opt_latency.size();
    idx = (size_t)(1.0 * size * p);
    printf("  [%.2fth]:%9lluns|(%9zu/%-9zu)\n", 100.0 * p, vec_opt_latency[idx], idx, size);
    return;
}

void do_readwrite(spdk_device_t* device, struct worker_options* options)
{
    int _io_depth = options->io_depth;
    int _num_read = (int)(options->read_ratio * _io_depth);
    int _num_write = _io_depth - _num_read;

    Timer _run_timer;
    uint64_t _time_ns = options->time_ns;
    std::vector<uint64_t> _read_latency;
    std::vector<uint64_t> _write_latency;

    size_t _read_bs = options->read_bs;
    size_t _write_bs = options->write_bs;
    uint32_t _read_lba = _read_bs / 512;
    uint32_t _write_lba = _write_bs / 512;

    struct spdk_nvme_qpair* _qpair = spdk_nvme_ctrlr_alloc_io_qpair(device->ctrlr, nullptr, 0);
    assert(_qpair != nullptr);

    uint32_t _read_base = 0;
    uint32_t _read_upper = _read_base + options->size / 512;
    uint32_t _write_base = 0;
    uint32_t _write_upper = _write_base + options->size / 512;

    uint32_t _read_pos = 0;
    uint32_t _write_pos = _write_base;

    _run_timer.Start();
    while (true) {
        int __c = 0;
        cb_t __cbs[64];
        // frist send read
        for (int i = 0; i < _num_read; i++) {
            __cbs[__c].timer.Start();
            char* __buff = (char*)spdk_zmalloc(_read_bs, _read_bs, nullptr, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_DMA);
            __cbs[__c].buff = __buff;
            int __rc = spdk_nvme_ns_cmd_read(device->ns, _qpair, __buff, _read_pos, _read_lba, read_cb, &__cbs[__c], 0);
            assert(__rc == 0);
            _read_pos += _read_lba;
            if (_read_pos >= _read_upper) {
                _read_pos = _read_base;
            }
            __c++;
        }
        // then send write
        for (int i = 0; i < _num_write; i++) {
            __cbs[__c].timer.Start();
            char* __buff = (char*)spdk_zmalloc(_write_bs, _write_bs, nullptr, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_DMA);
            __cbs[__c].buff = __buff;
            memset(__buff, 0xff, _write_bs);
            int __rc = spdk_nvme_ns_cmd_write(device->ns, _qpair, __buff, _write_pos, _write_lba, write_cb, &__cbs[__c], 0);
            assert(__rc == 0);
            _write_pos += _write_lba;
            if (_write_pos >= _write_upper) {
                _write_pos = _write_base;
            }
            __c++;
        }
        // waiting
        int __cnt = 0;
        while (true) {
            int __num = spdk_nvme_qpair_process_completions(_qpair, 0);
            __cnt += __num;
            if (__cnt == _io_depth) {
                break;
            }
        }
        // io depth
        for (int i = 0; i < _io_depth; i++) {
            if (i < _num_read) {
                _read_latency.push_back(__cbs[i].timer.Get());
            } else {
                _write_latency.push_back(__cbs[i].timer.Get());
            }
        }

        _run_timer.Stop();
        if (_run_timer.Get() > _time_ns) {
            break;
        }
    }
    spdk_nvme_ctrlr_free_io_qpair(_qpair);

    char _save_path[128];
    sprintf(_save_path, "%s/read.lat", g_save_path);
    result_output(_save_path, _read_latency);
    printf("---read---\n");
    sort(_read_latency.begin(), _read_latency.end());
    get_tail_latency(_read_latency, 0.99);
    get_tail_latency(_read_latency, 0.999);

    sprintf(_save_path, "%s/write.lat", g_save_path);
    result_output(_save_path, _write_latency);
    printf("---write---\n");
    sort(_write_latency.begin(), _write_latency.end());
    get_tail_latency(_write_latency, 0.99);
    get_tail_latency(_write_latency, 0.999);
}

int main(int argc, char** argv)
{
    time_t _t = time(NULL);
    struct tm* _lt = localtime(&_t);
    sprintf(g_save_path, "%04d%02d%02d_%02d%02d%02d", _lt->tm_year, _lt->tm_mon, _lt->tm_mday, _lt->tm_hour, _lt->tm_min, _lt->tm_sec);
    mkdir(g_save_path, 0777);

    int _res;
    spdk_device_t _dev;
    struct worker_options _options;
    _dev.transport_string = "trtype:PCIe traddr:0000:18:00.0";
    _res = spdk_nvme_transport_id_parse(&_dev.trid, _dev.transport_string.c_str());

    init_spdk_device(&_dev);
    printf("[%s-%zuGB]\n", _dev.transport_string.c_str(), _dev.ns_capacity / (1024 * 1024 * 1024));

    _options.io_depth = 16;
    _options.read_ratio = 0.2;
    _options.size = (size_t)16 * 1024 * 1024;
    _options.read_bs = 4096;
    _options.write_bs = 4096;
    _options.time_ns = (uint64_t)30 * 1000000000UL;

    do_readwrite(&_dev, &_options);

    spdk_nvme_detach(_dev.ctrlr);
    return 0;
}