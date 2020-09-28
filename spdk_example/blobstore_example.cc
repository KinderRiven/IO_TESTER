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

struct spdk_bdev* bdev;
struct spdk_bs_opts bs_opts;
struct spdk_bs_dev* bsdev;
struct spdk_app_opts app_opts;
struct spdk_blob* blos_store;

const char nvme_device[] = "Nvme0n1";

void bdev_init_cb(void* cb_arg, int rc)
{
    printf("bdev_init_cb[%d]\n", rc);
}

void bs_init_cb(void* cb_arg, struct spdk_blob_store* bs, int bserrno)
{
    printf("bs init finished!\n");
    printf("blobstore:0x%x\n", (uint64_t)bs);
    if (bs != nullptr) {
        printf("md_start:%llu, md_len:%d, io_unit:%d\n", bs->md_start, bs->md_len, bs->io_unit_size);
    }
}

void test_blobstore(void* cb)
{
    printf("[%s][%s]\n", app_opts.config_file, nvme_device);

    // get spdk bdev layer device
    bdev = spdk_bdev_get_by_name(nvme_device);
    if (bdev == nullptr) {
        printf("get bdev device failed!\n");
        exit(0);
    } else {
        printf("get bdev successful!\n");
    }

    // get blob device from bdev layer
    bsdev = spdk_bdev_create_bs_dev(bdev, nullptr, nullptr);
    if (bsdev == nullptr) {
        printf("get bsdev device failed!\n");
        exit(0);
    } else {
        printf("get bsdev successful!\n");
    }

    spdk_bs_init(bsdev, nullptr, bs_init_cb, nullptr);
    // spdk_bs_init(bsdev, &bs_opts, bs_init_cb, nullptr);
}

int main(int argc, char** argv)
{
    int rc;
    //spdk_bs_opts_init(&bs_opts);
    spdk_app_opts_init(&app_opts);
    app_opts.name = "blobstore_test";
    app_opts.config_file = "bdev.conf";
    rc = spdk_app_start(&app_opts, test_blobstore, nullptr);
    spdk_app_fini();
    return 0;
}