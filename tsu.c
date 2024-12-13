#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/version.h>


MODULE_AUTHOR("Lizza:)");
MODULE_DESCRIPTION("Tomsk State University Module");
MODULE_LICENSE("GPL");

static int __init tsu_init(void) {
    pr_info("Welcome to the Tomsk State University\n");
    return 0;
}

static void __exit tsu_exit(void) {
    pr_info("Tomsk State University forever!\n");
}

module_init(tsu_init);
module_exit(tsu_exit);

