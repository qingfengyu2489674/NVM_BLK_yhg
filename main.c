#include <linux/blk-mq.h>
#include <linux/errno.h>
#include <linux/genhd.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

#include "access.h"
#include "core.h"
#include "defs.h"
#include "format.h"
#include "io_handler.h"
#include "lower_dev.h"

#define CACHE_DEVICE_NAME "NVMCache"

static char *nvm_phy_addr_ranges = NULL;
module_param(nvm_phy_addr_ranges, charp, 0644);
MODULE_PARM_DESC(nvm_address_ranges,
                 "The NVM physical address ranges. "
                 "Format example: \"0x1000 0x2000 0x3000 0x4000\" "
                 "represents NVM has two physical address ranges [0x1000, "
                 "0x2000) and [0x3000, 0x4000)");

static unsigned long nvm_virt_addr = 0;
module_param(nvm_virt_addr, ulong, 0644);

static unsigned long nvm_virt_range_size = 0;
module_param(nvm_virt_range_size, ulong, 0644);

static char *lower_block_device_file_name = NULL;
module_param(lower_block_device_file_name, charp, 0644);
MODULE_PARM_DESC(lower_block_device_file_name, "The lower block device file, e.g. /dev/sda");

static int hw_queue_count = 1;
module_param(hw_queue_count, int, 0644);
MODULE_PARM_DESC(hw_queue_count, "Number of hardware queues");

typedef struct NVMCacheDevice {
    struct request_queue *queue;
    struct gendisk *gd;
    int major;
    struct blk_mq_tag_set tag_set;
    NvmCache *cache;
    NvmAccessor *accessor;
} NVMCacheDevice;

NVMCacheDevice nvm_dev;
struct block_device_operations nvm_dev_ops = {};

static blk_status_t nvm_cache_queue_rq(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data *bd) {
    struct request *req = bd->rq;
    blk_status_t status = BLK_STS_OK;
    struct bio_vec bvec;
    struct req_iterator iter;

    blk_mq_start_request(req);

    rq_for_each_segment(bvec, req, iter) {
        sector_t sector = iter.iter.bi_sector;
        char *buffer = kmap_atomic(bvec.bv_page);
        unsigned long offset = bvec.bv_offset;
        u64 sector_cnt = bvec.bv_len / CACHE_BLOCK_SIZE;  // 内核应保证bio segment大小是块设备扇区大小整数倍
        NvmCacheIoDir dir = bio_data_dir(iter.bio) == WRITE ? CACHE_WRITE : CACHE_READ;
        int ret;

        // TODO：
        // bio中sector大小单位是512字节
        // 注意这里req假设了CACHE_BLOCK_SIZE是512，如果不是，要进行转换！！！
        NvmCacheIoReq req = {.lba = sector,
                             .lba_num = sector_cnt,
                             .buffer = buffer + offset,
                             .dir = dir,
                             .resource_id = hctx->queue_num};

        // TODO: 可能睡眠。确认blk-mq的请求处理函数是否工作在进程上下文，如果不是，这里要用工作队列。
        ret = nvm_cache_handle_io(nvm_dev.cache, &req);
        if (ret) {
            kunmap_atomic(buffer);
            status = BLK_STS_IOERR;
            goto out;
        }

        kunmap_atomic(buffer);
    }

out:
    blk_mq_end_request(req, status);
    return status;
}

static struct blk_mq_ops nvm_mq_ops = {
    .queue_rq = nvm_cache_queue_rq,
};

static int create_block_device(NvmCache *cache, NvmAccessor *accessor) {
    int ret = 0;
    struct gendisk *gd;
    NvmCacheLowerDev *lower_bdev = cache->lower_bdev;

    ret = register_blkdev(0, CACHE_DEVICE_NAME);
    if (ret < 0) {
        pr_err("failed to alloc major number\n");
        goto ERR_OUT;
    }
    nvm_dev.major = ret;

    gd = alloc_disk(2);  // 1个分区
    if (!gd) {
        pr_err("failed to alloc disk\n");
        ret = -ENOMEM;
        goto ERR_REGISER_MAJOR;
    }

    nvm_dev.tag_set.ops = &nvm_mq_ops;
    nvm_dev.tag_set.nr_hw_queues = hw_queue_count;
    nvm_dev.tag_set.queue_depth = 128;  // 暂时写死队列深度，如有需要增加模块参数调整
    nvm_dev.tag_set.numa_node = NUMA_NO_NODE;
    nvm_dev.tag_set.cmd_size = 0;
    nvm_dev.tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
    nvm_dev.tag_set.driver_data = &nvm_dev;

    ret = blk_mq_alloc_tag_set(&nvm_dev.tag_set);
    if (ret) {
        pr_err("failed to allocate tag set\n");
        goto ERR_ALLOC_DISK;
    }

    nvm_dev.queue = blk_mq_init_queue(&nvm_dev.tag_set);
    if (IS_ERR(nvm_dev.queue)) {
        pr_err("failed to init queue\n");
        ret = PTR_ERR(nvm_dev.queue);
        goto ERR_TAG_SET;
    }

    blk_queue_physical_block_size(nvm_dev.queue, CACHE_BLOCK_SIZE);
    blk_queue_logical_block_size(nvm_dev.queue, CACHE_BLOCK_SIZE);

    gd->major = nvm_dev.major;
    gd->first_minor = 0;
    gd->fops = &nvm_dev_ops;
    gd->queue = nvm_dev.queue;
    gd->private_data = cache;
    snprintf(gd->disk_name, 32, CACHE_DEVICE_NAME);
    set_capacity(gd, lower_dev_get_sector_num(lower_bdev));  // 使用底层块设备的大小

    nvm_dev.gd = gd;

    nvm_dev.cache = cache;
    nvm_dev.accessor = accessor;

    add_disk(gd);

    return 0;

ERR_TAG_SET:
    blk_mq_free_tag_set(&nvm_dev.tag_set);
ERR_ALLOC_DISK:
    del_gendisk(gd);
ERR_REGISER_MAJOR:
    unregister_blkdev(nvm_dev.major, CACHE_DEVICE_NAME);
ERR_OUT:
    return ret;
}

static int __init nvm_cache_module_init(void) {
   


    // TODO:初始化工作
        // TODO:构建accessor,实现对NVM的按字节访问
        // TODO:构建核心数据结构 NVMCache

    // TODO:格式化结构，遍历NVM头部映射数组，依赖于核心数据结构的实现

    // 构建虚拟块设备

}

static void __exit nvm_cache_module_exit(void) {
    if (nvm_dev.gd) {
        del_gendisk(nvm_dev.gd);
        put_disk(nvm_dev.gd);
    }
    if (nvm_dev.queue) {
        blk_cleanup_queue(nvm_dev.queue);
    }
    blk_mq_free_tag_set(&nvm_dev.tag_set);
    unregister_blkdev(nvm_dev.major, CACHE_DEVICE_NAME);

    nvm_cache_destruct(nvm_dev.cache);
    kfree(nvm_dev.cache);
    nvm_accessor_destruct(nvm_dev.accessor);
    kfree(nvm_dev.accessor);

    pr_info("nvm cache module exited.\n");
}

module_init(nvm_cache_module_init);
module_exit(nvm_cache_module_exit);
MODULE_LICENSE("GPL");
