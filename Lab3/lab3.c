#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#define procfs_name "lab3"
static struct proc_dir_entry *proc_file = NULL;

// функции загрузки и выгрузки модуля (используются как псевдонимы для init_module()/cleanup_module()
static int __init procfs_init(void){
	pr_info("Welcome to the Tomsk State University (3 years and coming!)\n");
	return 0;
}
static void __exit procfs_exit(void){
	pr_info("Tomsk State University forever!\n");
}
module_init(procfs_init);
module_exit(procfs_exit);
MODULE_LICENSE("GPL");

