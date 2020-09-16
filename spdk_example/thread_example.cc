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
    printf("START!\n");
    while (true) {
    }
}

int f_parese(int f, char* arg)
{
    printf("parese!\n");
    return 0;
}

void f_usage()
{
    printf("usage!\n");
}

int main(int argc, char** argv)
{
    int rc;
    struct spdk_app_opts opts = {};
    opts.name = "bdev-example";

    spdk_app_opts_init(&opts);

    // if ((rc = spdk_app_parse_args(argc, argv, &opts, "b:", NULL, f_parese, f_usage)) != SPDK_APP_PARSE_ARGS_SUCCESS) {
    //     printf("ERRROR!\n");
    //     exit(rc);
    // }

    spdk_app_start(&opts, start_thread, nullptr);
    return 0;
}