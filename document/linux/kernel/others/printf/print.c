#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/tty.h>

MODULE_LICENSE("GPL");
static void _printf(char *str)
{
	struct tty_struct *my_tty;
	my_tty = current->signal->tty;
	if (my_tty != NULL)
	{
		my_tty->ops->write(my_tty, str,strlen(str));
		my_tty->ops->write(my_tty, "\015\013", 2);
	}
}

static int printf(const char *format, ...)
{
	char buf[1000];
	va_list ap;

	va_start(ap, format);    
	vsprintf(buf, format, ap); 
	va_end(ap);
	
	_printf(buf);

	return 0;
}

static int __init print_string_init(void)
{	
	int a = 7;
	printf("Hello world: %d", a);
	printf("Hello  %d", a);
	printf("s");

	return 0;
}
static void __exit print_string_exit(void)
{
    	printf("Goodbye world!");
}

module_init(print_string_init);
module_exit(print_string_exit);

