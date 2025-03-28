#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>
#include <linux/bio.h>

#include "defs.h"
#include "lower_dev.h"

// 用于打开底层块设备
#define OPEN_MODE (FMODE_READ | FMODE_WRITE | FMODE_EXCL)

int lower_dev_init(NvmCacheLowerDev *dev, const char *dev_name) {
    // note: FMODE_EXCL: 以独占模式打开，holder参数相当于拥有者的key，同一拥有者可以递归打开
    dev->bd = blkdev_get_by_path(dev_name, OPEN_MODE, dev);
    if (IS_ERR(dev->bd)) {
        pr_err("failed to open %s\n", dev_name);
        return PTR_ERR(dev->bd);
    }
    dev->dev_name = dev_name;
    return 0;
}

void lower_dev_destruct(NvmCacheLowerDev *self) {
    blkdev_put(self->bd, OPEN_MODE);
}

u64 lower_dev_get_sector_num(NvmCacheLowerDev *self) {
    return self->bd->bd_part->nr_sects;
}

int lower_dev_io(NvmCacheLowerDev *self, LowerDevIoReq *req) {
    struct bio *bio;
    struct page *page;
    unsigned int len, offset;
    int ret;

    bio = bio_alloc(GFP_KERNEL, 1);
    if (IS_ERR(bio)) {
        pr_err("failed to allocate bio\n");
        return PTR_ERR(bio);
    }

    bio->bi_iter.bi_sector = req->sector;
    bio_set_dev(bio, self->bd);
    bio->bi_end_io = NULL;
    bio->bi_opf = req->dir == CACHE_WRITE ? REQ_OP_WRITE : REQ_OP_READ;

    page = virt_to_page(req->buffer);
    len = req->sector_num * 512;
    offset = offset_in_page(req->buffer);
    bio_add_page(bio, page, len, offset);

    ret = submit_bio_wait(bio);
    if (ret) {
        pr_err("bio submission failed\n");
    }

    bio_put(bio);

    return ret;
}