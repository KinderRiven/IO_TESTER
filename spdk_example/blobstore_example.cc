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
    printf("[0]bdev_init_cb[%d]\n", rc);
}

// 5.
static void blob_resize_cb(void* cb_arg, int bserrno)
{
    struct spdk_blob* blob = (struct spdk_blob*)cb_arg;
    printf("[5]blob resize finished!(%d)\n", bserrno);
    uint64_t total = spdk_blob_get_num_clusters(blob);
    printf("[5]blob cluster count:%llu\n", total);
}

// 4.
static void open_blob_cb(void* cb_arg, struct spdk_blob* blob, int bserrno)
{
    struct spdk_blob_store* bs = (struct spdk_blob_store*)cb_arg;
    int free_count = 0;
    printf("[4]blob open finished!(%d)(0x%llx)\n", bserrno, (uint64_t)blob);
    free_count = spdk_bs_free_cluster_count(bs);
    printf("[4]blob free cluster_count (%d)\n", free_count);
    spdk_blob_resize(blob, free_count, blob_resize_cb, (void*)blob);
}

// 3. Open a new Blob in BlobStore
static void blob_create_cb(void* arg1, spdk_blob_id blobid, int bserrno)
{
    printf("[3]blob create finished!(%d)\n", bserrno);
    struct spdk_blob_store* bs = (struct spdk_blob_store*)arg1;
    spdk_bs_open_blob(bs, blobid, open_blob_cb, arg1);
}

// 2. Create Blob from BlobStore
void bs_init_cb(void* cb_arg, struct spdk_blob_store* bs, int bserrno)
{
    // We has create a blobstore, then we need to create a blob in blobstore
    uint64_t io_usize = spdk_bs_get_io_unit_size(bs);
    printf("[2]blobstore create&init finished!(%d)\n", bserrno);
    printf("[2]blobstore:0x%x(%lluB)\n", (uint64_t)bs, io_usize);
    // Create a new blob with default option values on the given blobstore.
    // The new blob id will be passed to the callback function.
    spdk_bs_create_blob(bs, blob_create_cb, (void*)bs);
}

// 1. Create BlobStore from Bdev
void test_blobstore(void* cb)
{
    printf("[%s][%s]\n", app_opts.config_file, nvme_device);

    // get spdk bdev layer device
    bdev = spdk_bdev_get_by_name(nvme_device);
    if (bdev == nullptr) {
        printf("[1]get bdev device failed!\n");
        exit(0);
    } else {
        printf("[1]get bdev successful!\n");
    }

    // bs = [b]lock[s]tore
    // Create a blobstore block device from a bdev. (deprecated, please use spdk_bdev_create_bs_dev_from_desc,
    // together with spdk_bdev_open_ext).
    bsdev = spdk_bdev_create_bs_dev(bdev, nullptr, nullptr);
    if (bsdev == nullptr) {
        printf("[1]get bsdev device failed!\n");
        exit(0);
    } else {
        printf("[1]get bsdev successful!\n");
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