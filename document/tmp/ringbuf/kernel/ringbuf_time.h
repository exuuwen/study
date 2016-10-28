#ifndef __RINGBUF_TIME_H__
#define __RINGBUF_TIME_H__

int rt_create_table(void);
void rt_remove_table(void);
int rt_set_offset(int offset);
int rt_get_time(char *date_string, unsigned int raw_time);

#endif /*__RINGBUF_TIME_H__*/
