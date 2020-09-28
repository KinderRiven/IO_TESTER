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

#include "spdk/blob.h"
#include "spdk/blob_bdev.h"
#include "spdk/blobfs.h"
#include "spdk/blobfs_bdev.h"

int main(int argc, char** argv)
{
    struct spdk_bdev* bdev;
    struct spdk_bs_opts opts;
    struct spdk_bs_dev* bsdev;

    bdev = spdk_bdev_get_by_name("NVMe0");
    if (bdev == nullptr) {
        printf("get bdev device failed!\n");
        exit(0);
    }

    spdk_bs_opts_init(&opts);
    
    bsdev = spdk_bdev_create_bs_dev(bdev, nullptr, nullptr);
    if (bsdev == nullptr) {
        printf("get bsdev device failed!\n");
        exit(0);
    }
    return 0;
}