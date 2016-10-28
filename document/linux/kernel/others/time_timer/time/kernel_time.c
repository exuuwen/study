#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/time.h> 
#include <linux/mm.h>
#include <linux/delay.h>

int time_module_init() {
 
	struct timeval tv_start, tv_end; 
	 
	do_gettimeofday(&tv_start);
	//printk("the second now is %ld\n",tv->tv_sec);
	//printk("the micro second is %ld\n",tv->tv_usec); 
	msleep(800);
	do_gettimeofday(&tv_end);

	printk("the second diff is %ld, end is %ld, start is %ld\n",tv_end.tv_sec - tv_start.tv_sec, tv_end.tv_sec, tv_start.tv_sec);
	printk("the mcro second diff is %ld, end is %ld, start is %ld\n",tv_end.tv_usec - tv_start.tv_usec, tv_end.tv_usec, tv_start.tv_usec);
	printk("the  diff time is %ld\n", (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + (tv_end.tv_usec - tv_start.tv_usec));
	return 0; 
} 

void time_module_exit() { 
	printk("exit the time test module\n"); 

} 

module_init(time_module_init);
module_exit(time_module_exit);


