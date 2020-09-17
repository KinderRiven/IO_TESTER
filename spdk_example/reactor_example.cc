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

#include "spdk/bdev.h"
#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/nvme.h"
#include "spdk/stdinc.h"
#include "spdk/vmd.h"

char* g_bdev_name;

struct app_msg_t {
public:
    char* bdev_name;
};

void write_cb(struct spdk_bdev_io* bdev_io, bool success, void* cb_arg)
{
    printf("[write_callback:%d]\n", success);
}

int tick_f1(void* num)
{
    uint64_t p = *((uint64_t*)num);
    printf("[tick_f1:%llu]\n", p);
    return 1;
}

int tick_f2(void* num)
{
    uint64_t p = *((uint64_t*)num);
    printf("[tick_f2:%llu]\n", p);
    return 1;
}

void start_app(void* cb)
{
    printf("start_app!\n");

    uint64_t* tick_1 = (uint64_t*)malloc(sizeof(uint64_t));
    *tick_1 = 500000;
    printf("poller_register (1)!\n");
    struct spdk_poller* poller_1 = spdk_poller_register(tick_f1, (void*)tick_1, *tick_1);

    uint64_t* tick_2 = (uint64_t*)malloc(sizeof(uint64_t));
    *tick_2 = 5000000;
    printf("poller_register (2)!\n");
    struct spdk_poller* poller_2 = spdk_poller_register(tick_f2, (void*)tick_2, *tick_2);
}

int bdev_parse_arg(int ch, char* arg)
{
    switch (ch) {
    case 'b':
        printf(">>>>[bdev_name(%c)-(%s)]\n", ch, arg);
        g_bdev_name = arg;
        break;
    default:
        break;
    }
    return 0;
}

void bdev_usage()
{
    printf(">>>>[bdev_usage]\n");
}

int main(int argc, char** argv)
{
    int rc;
    struct app_msg_t app_msg;
    struct spdk_app_opts opts = {};

    spdk_app_opts_init(&opts);
    opts.name = "bdev-example";

    if ((rc = spdk_app_parse_args(argc, argv, &opts, "b:", nullptr, bdev_parse_arg, bdev_usage)) != SPDK_APP_PARSE_ARGS_SUCCESS) {
        printf(">>>>[spdk_app_parse_arg error!]\n");
        exit(rc);
    }

    app_msg.bdev_name = g_bdev_name;
    printf("OPT [name:%s][file_name:%s][reactor_mask:%s][master_core:%d]\n", opts.name, opts.config_file, opts.reactor_mask, opts.master_core);
    printf("APP [name:%s]\n", app_msg.bdev_name);
    rc = spdk_app_start(&opts, start_app, (void*)&app_msg);
    spdk_app_stop(rc);
    return 0;
}