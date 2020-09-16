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

void start_thread(void* cb)
{
    printf("start_thread (0x%llx)\n", (uint64_t)cb);
}

void bdev_parse_arg(int ch, char* arg)
{
    printf("bdev_parse_arg (%d) (%s)\n", ch, arg);
}

void bdev_usage()
{
    printf("bdev_usage\n");
}

int main(int argc, char** argv)
{
    int rc;
    struct spdk_app_opts opts = {};
    opts.name = "bdev-example";
    hello_context.bdev_name = "sixsixsix";

    spdk_app_opts_init(&opts);

    if ((rc = spdk_app_parse_args(argc, argv, &opts, "b:", NULL, bdev_parse_arg, bdev_usage)) != SPDK_APP_PARSE_ARGS_SUCCESS) {
        printf("spdk_app_parse_arg error!\n");
        exit(rc);
    }

    rc = spdk_app_start(&opts, start_thread, nullptr);
    spdk_app_stop(rc);
    return 0;
}