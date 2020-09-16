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
#include "spdk/event.h"
#include "spdk/nvme.h"
#include "spdk/stdinc.h"
#include "spdk/vmd.h"

char* g_bdev_name;

struct app_msg_t {
public:
    char* bdev_name;
};

void start_app(void* cb)
{
    struct app_msg_t* msg = (struct app_msg_t*)cb;
    struct spdk_bdev* bdev;

    printf(">>>>[start_thread(0x%llx)]\n", (uint64_t)cb);

    bdev = spdk_bdev_get_by_name(msg->bdev_name);
    printf("    [bdev:%s|%llx]\n", msg->bdev_name, (uint64_t)bdev);
}

int bdev_parse_arg(int ch, char* arg)
{
    printf(">>>>[bdev_parse_arg(%c)-(%s)]\n", ch, arg);
    g_bdev_name = arg;
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

    if ((rc = spdk_app_parse_args(argc, argv, &opts, "b:", NULL, bdev_parse_arg, bdev_usage)) != SPDK_APP_PARSE_ARGS_SUCCESS) {
        printf(">>>>[spdk_app_parse_arg error!]\n");
        exit(rc);
    }

    app_msg.bdev_name = g_bdev_name;
    printf("OPT [name:%s][file_name:%s]\n", opts.name, opts.config_file);
    printf("APP [name:%s]\n", app_msg.bdev_name);
    rc = spdk_app_start(&opts, start_app, nullptr);

    spdk_app_stop(rc);
    return 0;
}