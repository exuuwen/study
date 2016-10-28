#include <syslog.h>
#include <stdio.h>
#include <ev.h>
#include <stdint.h>

uint64_t count;

static void
timer_repeat_cb (struct ev_loop *loop, ev_timer *w, int revents)
{
	fprintf(stderr, "hello world stderr %u\n", count);
	syslog(LOG_INFO, "hello world syslog %u\n", count);
	count++;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "./foo param\n");
		return -1;
	}

	openlog("foo", LOG_PID, LOG_USER);
	fprintf(stderr, "hello world stderr %s\n", argv[1]);
	syslog(LOG_INFO, "hello world syslog %s\n", argv[1]);

	struct ev_loop * loop;

	loop = ev_default_loop(0);

	struct ev_timer timer_repeat_watcher;
	ev_timer_init(&timer_repeat_watcher, timer_repeat_cb, 10, 10);
	ev_timer_start (loop, &timer_repeat_watcher);

	ev_run (loop, 0);
}
