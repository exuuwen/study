#ifndef __UTC_TIME_H__
#define __UTC_TIME_H__

int rt_create_utc(void);
void rt_remove_utc(void);
int rt_set_utc_offset(int offset);
int rt_get_time(char *date_string, unsigned int raw_time);

#endif /*__UTC_TIME_H__*/
