#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ghali Boucetta <gboucett@student.42.fr>");
MODULE_DESCRIPTION("A simple Hello World module");
MODULE_VERSION("0.1");

int my_init(void)
{
	printk(KERN_INFO "FROM ASSIGNMENT04 MODULE: USB keyboard inserted...\n");
	return 0;
}

void my_exit(void)
{
	printk(KERN_INFO "Goodbye, cruel world\n");
}

module_init(my_init);
module_exit(my_exit);
