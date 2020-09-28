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

int rc;
struct spdk_bdev* bdev;
struct spdk_bs_opts bs_opts;
struct spdk_bs_dev* bsdev;
struct spdk_app_opts app_opts;

void test_blobstore(void* cb)
{
    bdev = spdk_bdev_get_by_name("NVMe0");
    if (bdev == nullptr) {
        printf("get bdev device failed!\n");
        exit(0);
    }

    bsdev = spdk_bdev_create_bs_dev(bdev, nullptr, nullptr);
    if (bsdev == nullptr) {
        printf("get bsdev device failed!\n");
        exit(0);
    }
}

int main(int argc, char** argv)
{
    spdk_app_opts_init(&app_opts);
    rc = spdk_app_parse_args(argc, argv, &app_opts, "b:", nullptr, nullptr, nullptr);

    spdk_bs_opts_init(&bs_opts);
    spdk_app_start(app_opts, test_blobstore, nullptr);
    return 0;
}