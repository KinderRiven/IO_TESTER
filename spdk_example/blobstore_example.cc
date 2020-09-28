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

const char* nvme_device[3] = { "Nvme0n1", "Nvme1n1", "Nvme2n1" };

void bdev_init_cb(void* cb_arg, int rc)
{
    printf("bdev_init_cb[%d]\n", rc);
}

void bs_init_cb(void* cb_arg, struct spdk_blob_store* bs, int bserrno)
{
    printf("bs init finished!\n");
}

void test_blobstore(void* cb)
{
    printf("[%s][%s]\n", app_opts.config_file, nvme_device[0]);
    bdev = spdk_bdev_get_by_name(nvme_device[0]);

    if (bdev == NULL) {
        printf("get bdev device failed!\n");
        exit(0);
    } else {
        printf("get bdev successful!\n");
    }

    bsdev = spdk_bdev_create_bs_dev(bdev, NULL, NULL);

    if (bsdev == NULL) {
        printf("get bsdev device failed!\n");
        exit(0);
    } else {
        printf("get bsdev successful!\n");
    }
    spdk_bs_init(bsdev, NULL, bs_init_cb, NULL);
    // spdk_bs_init(bsdev, &bs_opts, bs_init_cb, NULL);
}

int main(int argc, char** argv)
{
    int rc;
    //spdk_bs_opts_init(&bs_opts);
    spdk_app_opts_init(&app_opts);
    app_opts.name = "blobstore_test";
    app_opts.config_file = "bdev.conf";
    rc = spdk_app_start(&app_opts, test_blobstore, NULL);
    spdk_app_fini();
    return 0;
}