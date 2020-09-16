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

#include "spdk/event.h"
#include "spdk/env.h"
#include "spdk/nvme.h"
#include "spdk/stdinc.h"
#include "spdk/vmd.h"

void* start_thread(void* cb)
{
    printf("START!\n");
    while (true) {
    }
}

int main()
{
    struct spdk_app_opts opts;
    spdk_app_start(&opts, start_thread, nullptr);
    return 0;
}