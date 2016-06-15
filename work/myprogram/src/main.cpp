#include <stdio.h>
#include <ev.h>
#include "mylib.h"
#include "unetanalysis.177000.178000.pb.h"

extern void haha(int a);

static void
timer_repeat_cb (struct ev_loop *loop, ev_timer *w, int revents)
{
	printf("timer repeat out %d\n", revents);
}

int main(void)
{
	printf("hello world\n");
	mylib_func(10);
	haha(100);

	struct ev_loop * loop;

	loop = ev_default_loop(0);

	struct ev_timer timer_repeat_watcher;
	ev_timer_init(&timer_repeat_watcher, timer_repeat_cb, 2, 2);
	ev_timer_start (loop, &timer_repeat_watcher);

	ev_run (loop, 0);


	
	return 0;
}
