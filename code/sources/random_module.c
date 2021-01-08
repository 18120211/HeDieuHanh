#include <linux/module.h>   
#include <linux/random.h>   //get_random_bytes(&var, num_bytes);
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define MOD_NAME "random_module"
#define DEV_FILE_NAME "random_number"
#define DRIVER_AUTHOR "Ngo Dai Son <ngodaisonn@gmail.com>"
#define DRIVER_DESC "Randomized a number module"

static dev_t first; // Global variable for the first device number
static struct cdev c_dev; // Global variable for the character device structure
static struct class *cl; // Global variable for the device class

static int my_open(struct inode *i, struct file *f)
{
    printk(KERN_INFO "Driver: open()\n");
    return 0;
}

static int my_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "Driver: close()\n");
    return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    int ans;
    get_random_bytes(&ans, sizeof(ans));

    if(!copy_to_user(buf, &ans, sizeof(ans))){   
        printk(KERN_INFO "Driver: return randomized number to user\n");
        return 0;
    }
    else{
        printk(KERN_INFO "Sending randomized number to user failed\n");
        return -1;
    }
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Driver: write()\n");
    return len;
}

static struct file_operations pugs_fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write
};

static int __init randnum_init(void) /* Constructor */
{
    printk(KERN_INFO "RN: %s registered", MOD_NAME);
    if (alloc_chrdev_region(&first, 0, 1, MOD_NAME) < 0)
    {
        printk(KERN_ALERT "RN: Major number registration failed\n");
        return -1;
    }
    printk(KERN_INFO "<Major, Minor>: <%d, %d>\n", MAJOR(first), MINOR(first));

    if ((cl = class_create(THIS_MODULE, DEV_FILE_NAME)) == NULL)
    {
        unregister_chrdev_region(first, 1);
        printk(KERN_ALERT "RN: Device class registration failed\n");
        return -1;
    }
    printk(KERN_INFO "RN: Device class's successfully registered\n");

    if (device_create(cl, NULL, first, NULL, MOD_NAME) == NULL)
    {
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return -1;
    }
    cdev_init(&c_dev, &pugs_fops);
    if (cdev_add(&c_dev, first, 1) == -1)
    {
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return -1;
    }
    return 0;
}

static void __exit randnum_exit(void) /* Destructor */
{
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    printk(KERN_INFO "RN: unregistered");
}

module_init(randnum_init);
module_exit(randnum_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
