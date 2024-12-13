#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/time.h>

#define procfs_name "lab3"
static struct proc_dir_entry *proc_file = NULL;
// структура, в которую будем записывать время последнего чтения
time64_t last_read_time = -1;

// функция, формирующее содержимое файла при попытке чтения
static ssize_t procfile_read(
  struct file *file_pointer,
  char __user *buffer,
  size_t buffer_length,
  loff_t *offset)
{
	// записываем время открытия файла на чтение
	time64_t cur_read_time = ktime_get_seconds();
	time64_t delta = 0;
	if(last_read_time != -1){
		delta = cur_read_time - last_read_time;
		delta %= 60;
	}
	// вычисление угла поворота секундной стрелки
	delta = delta * 6;
	last_read_time = cur_read_time;
	char str[4];
	snprintf(str, 4, "%d", delta);
	int n = 1;
	while(delta >= 10) {
		delta /= 10;
		n += 1;
	}

	// запись в файл
	int len = n;
	ssize_t ret = len;

	if(*offset >= len || copy_to_user(buffer, str, len)){
		ret = 0;
	} else {
		*offset += len;
	}
	pr_info("%s\n", str);
	return ret;
}
static const struct proc_ops proc_file_fops = {
	.proc_read = procfile_read,
};
// функции загрузки и выгрузки модуля (используются как псевдонимы для init_module()/cleanup_module()
static int __init procfs_init(void){
	proc_file = proc_create(procfs_name, 0644, NULL, &proc_file_fops);
	pr_info("Welcome to the Tomsk State University (3 years and coming!)\n");
	return 0;
}
static void __exit procfs_exit(void){
	pr_info("Tomsk State University forever!\n");
	proc_remove(proc_file);
}
module_init(procfs_init);
module_exit(procfs_exit);
MODULE_LICENSE("GPL");

