#include "ringbuf_time.h"

#ifdef USER_SPACE
/** User space specific include files and definitions to emulate kernel space*/
#  include <stdio.h>
#  include <stdlib.h>
#  include <errno.h>
#  include <time.h>
#  include <inttypes.h>
#  define kmalloc(size,flags) malloc(size)
#  define kfree(ptr) free(ptr)
#  define printk(fmt,args...) fprintf(stderr,fmt,##args)
#  define	KERN_EMERG	"<0>"	/* system is unusable	*/
#  define	KERN_ALERT	"<1>"	/* action must be taken immediately*/
#  define	KERN_CRIT	"<2>"	/* critical conditions	*/
#  define	KERN_ERR	"<3>"	/* error conditions	*/
#  define	KERN_WARNING	"<4>"	/* warning conditions	*/
#  define	KERN_NOTICE	"<5>"	/* normal but significant condition */
#  define	KERN_INFO	"<6>"	/* informational        */
#  define	KERN_DEBUG	"<7>"	/* debug-level messages	*/
/****************** End of User space specifics ******************************/
#else
/*Kernel space specific include files */
#  include <linux/time.h>
#  include <linux/types.h>
#  include <linux/kernel.h>
#  include <linux/slab.h>
#  include <linux/err.h>
#endif
/****************** End of kernel space specifics ****************************/

/* Seconds since jan 1 1970 for diffentent months from
 * january 2001 to december 2037 */
static const int START_YEAR=2001;
static const int END_YEAR=2037;
static const int SECONDS_AT_JANUARY_2001=978307200;
static const int SECONDS_PER_DAY=24*3600;
static int* g_p_seconds;
static int g_offset;

/* Generated bu doing:
 * for i in range(1,13):
 *   datetime.date(2007, i,1) - datetime.date(2007, 1,1)
 *
 * in a python shell :-) */
static int days_to_month[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

int rt_create_table(void)
{
    int year, month, base, seconds;
    
    g_p_seconds = (int*)kmalloc((END_YEAR-START_YEAR+1)*12*sizeof(int), GFP_KERNEL);
    if (NULL == g_p_seconds) {
	printk(KERN_ERR "Error, cannot allocate memory\n");
	return -ENOMEM;
    }
  
    base = SECONDS_AT_JANUARY_2001;
    for (year=START_YEAR; year<=END_YEAR; year++) {
	for (month=0; month<12; month++) {
	    /* Calculate seconds until this month */
	    seconds = base+days_to_month[month]*SECONDS_PER_DAY;
	    /* Add for leap year if March or later */
	    if ((0 == year%4) && (1 < month)) seconds += SECONDS_PER_DAY;
	    g_p_seconds[(year-START_YEAR)*12+month] = seconds;
	}
	/* The base for next year is current seconds + december */
	base = seconds+31*SECONDS_PER_DAY;
    }
    return 0;
}

void rt_remove_table(void)
{
    if (NULL != g_p_seconds) {
	kfree(g_p_seconds);
	g_p_seconds = NULL;
    }
}

int rt_set_offset(int offset)
{
    if (abs(offset) > SECONDS_PER_DAY) return -EINVAL;
    g_offset = offset;
    return 0;
}

int rt_get_time(char *date_string, unsigned int raw_time)
{
    int time_stamp = raw_time - g_offset;
    int year, month, previous_month_seconds, current_month_seconds;
    int day, time, hour, minute, second;

    if (time_stamp < SECONDS_AT_JANUARY_2001) {
	sprintf(date_string, "2001-01-01 00:00:00");
	printk(KERN_WARNING "Got too low time stamp %d\n", time_stamp);
	printk(KERN_WARNING "Replying with %s\n", date_string);
	return -EINVAL;
    }
    if (time_stamp > g_p_seconds[(END_YEAR-START_YEAR+1)*12-1]) {
	sprintf(date_string, "%d-12-31 23:59:59", END_YEAR);
	printk(KERN_WARNING "Got too high time stamp %d\n", time_stamp);
	printk(KERN_WARNING "Replying with %s\n", date_string);
	return -EINVAL;
    }
    previous_month_seconds = SECONDS_AT_JANUARY_2001;
    for (year=START_YEAR; year<=END_YEAR; year++) {
	for (month=0; month<12; month++) {
	    current_month_seconds = g_p_seconds[(year-START_YEAR)*12+month];
	    /* If current month seconds is more than time stamp use previous month. */
	    if (current_month_seconds > time_stamp) {
		/* If we are in january, go back to december last year,
		 * otherwize the month index is ok. */
		if (0 == month) {
		    year--;
		    month = 12;
		}
		day = (time_stamp-previous_month_seconds) / SECONDS_PER_DAY;
		/* The first day in a month is actually 1 not 0 */
		day++;
		time = (time_stamp-previous_month_seconds) % SECONDS_PER_DAY;
		hour = time / 3600;
		minute = (time / 60) % 60;
		second = time % 60;
		sprintf(date_string, "%04d-%02d-%02d %02d:%02d:%02d",
			year, month, day, hour, minute, second);
		return 0;
	    }
	    previous_month_seconds = current_month_seconds;
	}
    }
    /* Should never come here */
    return -EINVAL;
}
/* _______________________________________________________________________ */
/*
  Local Variables:
  mode: C
  c-basic-offset: 4
  End:
*/
