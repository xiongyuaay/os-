#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/spinlock.h>

// 相关操作函数声明
static int dev_open(struct inode *inode, struct file *filp);
static ssize_t dev_read(struct file *filp, char __user *buf, size_t size, loff_t *pos);
static ssize_t dev_write(struct file *filp, const char __user *buf, size_t size, loff_t *pos);
static loff_t dev_llseek(struct file *filp, loff_t offset, int whence);
static int dev_release(struct inode *inode, struct file *filp);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yuaay");
#define MAJOR_NUM 290
#define DEVICE_NAME "mydev"
#define DEV_SIZE 1024

#define MAX_MSG_LEN 256
#define MAX_MSG_COUNT 64

struct Message {
    pid_t sender_pid;    // 发送者进程 ID
    char content[MAX_MSG_LEN]; // 消息内容
};

struct MessageQueue {
    struct Message messages[MAX_MSG_COUNT];
    int head;             // 队列头
    int tail;             // 队列尾
    spinlock_t lock;      // 自旋锁保护队列
    int count;            // 队列中消息数量
};

// 定义设备存储块结构体，使用消息队列定义
struct Device {
    struct MessageQueue queue;
};

// 驱动子设备结构体指针
struct Device *device;

// 文件操作结构体
struct file_operations dev_fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .write = dev_write,
    .open = dev_open,
    .llseek = dev_llseek,
    .release = dev_release
};

// 模块初始化函数
static int __init init_mymodule(void) {
    int result;

    // 注册字符设备
    result = register_chrdev(MAJOR_NUM, DEVICE_NAME, &dev_fops);
    if (result < 0) {
        printk(KERN_ERR "Failed to register character device\n");
        return result;
    }

    // 分配设备结构体
    device = kmalloc(sizeof(struct Device), GFP_KERNEL);
    if (!device) {
        unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
        return -ENOMEM;
    }


    device->size = DEV_SIZE;
    device->data = kmalloc(DEV_SIZE, GFP_KERNEL);
    if (!device->data) {
        kfree(device);
        unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
        return -ENOMEM;
    }

    memset(device->data, 0, DEV_SIZE);
    printk(KERN_INFO "Device initialized successfully\n");
    return 0;
}

// 模块清理函数
static void __exit cleanup_mymodule(void) 
{
    if (device) {
        if (device->data)
            kfree(device->data);
        kfree(device);
    }
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk(KERN_INFO "Device unregistered successfully\n");
}

static int dev_open(struct inode *inode, struct file *filp) {
    filp->private_data = device;
    return 0;
}

static ssize_t dev_read(struct file *filp, char __user *buf, size_t size, loff_t *pos) {
    struct Device *dev = filp->private_data;
    size_t to_read;

    if (*pos >= DEV_SIZE)
        return 0;

    to_read = min(size, (size_t)(DEV_SIZE - *pos));
    if (copy_to_user(buf, dev->data + *pos, to_read))
        return -EFAULT;

    *pos += to_read;
    return to_read;
}

static ssize_t dev_write(struct file *filp, const char __user *buf, size_t size, loff_t *pos) {
    struct Device *dev = filp->private_data;
    size_t to_write;

    if (*pos >= DEV_SIZE)
        return 0;

    to_write = min(size, (size_t)(DEV_SIZE - *pos));
    if (copy_from_user(dev->data + *pos, buf, to_write))
        return -EFAULT;

    *pos += to_write;
    return to_write;
}

static loff_t dev_llseek(struct file *filp, loff_t offset, int whence) {
    loff_t newpos;

    switch (whence) {
    case SEEK_SET:
        newpos = offset;
        break;
    case SEEK_CUR:
        newpos = filp->f_pos + offset;
        break;
    case SEEK_END:
        newpos = DEV_SIZE + offset;
        break;
    default:
        return -EINVAL;
    }

    if (newpos < 0 || newpos > DEV_SIZE)
        return -EINVAL;

    filp->f_pos = newpos;
    return newpos;
}

static int dev_release(struct inode *inode, struct file *filp) {
    return 0;
}

module_init(init_mymodule);
module_exit(cleanup_mymodule);
