#include <stdio.h>
#include <ev.h>
#include <unistd.h>
#include <math.h>

static struct ev_timer timer_watcher;
static struct ev_timer timer_new_watcher;

static void
stdin_cb (struct ev_loop *loop, ev_io *w, int revents)
{
 	ev_io_stop(loop, w);
	printf("revents %d, fd %d\n", revents, w->fd);

	//both stop and again will stop non-repeat timer
	//ev_timer_stop(loop, &timer_watcher);
	ev_timer_again(loop, &timer_watcher);
}

static void
timer_repeat_cb (struct ev_loop *loop, ev_timer *w, int revents)
{
	printf("timer repeat out %d\n", revents);
}

static int n_timer = 0;
static void
timer_cb (struct ev_loop *loop, ev_timer *w, int revents)
{
	printf("timer out %d\n", revents);
	if (n_timer < 5)
	{
		ev_timer_set(w, 5, 0);
		ev_timer_start(loop, w);
		n_timer++;
	}
}

static int n = 0;
static void
timer_new_cb (struct ev_loop *loop, ev_timer *w, int revents)
{
	printf("timer new out %d\n", revents);
	if (n == 0)
	{
		n = 1;
		//ev_timer_set(w, 0, 2);
		w->repeat = 2;
		ev_timer_again(loop, w);
	}
	else if(n < 5)
	{
		n++;
	}
	else
		ev_timer_stop(loop, w);
}

static void
periodic_repeat_cb (struct ev_loop *loop, ev_timer *w, int revents)
{
    printf("periodic repeat out %d\n", revents);
}

static void
periodic_cb (struct ev_loop *loop, ev_timer *w, int revents)
{
    printf("periodic out %d\n", revents);
}

static void
periodic_resch_cb (struct ev_loop *loop, ev_timer *w, int revents)
{
    printf("periodic resch timeout\n");
}

static ev_tstamp scheduler_cb (ev_periodic *w, ev_tstamp now)
{
    //整30s一分钟里的(0或者30s)
    return now + (30 - fmod (now, 30));
    //下个30s
    //return now + 30;
}

static void
signal_cb (struct ev_loop *loop, ev_signal *w, int revents)
{
	printf("signal int %d\n", revents);
	ev_break(loop, EVBREAK_ALL);
}


int main()
{
	struct ev_loop * loop;

	loop = ev_default_loop(0);

	struct ev_io stdin_watcher;
	ev_init(&stdin_watcher, stdin_cb);
	ev_io_set(&stdin_watcher, STDIN_FILENO, EV_READ);
	ev_io_start(loop, &stdin_watcher);

	struct ev_timer timer_repeat_watcher;
	//第一次从offset开始 如果offset为0 立即开始
	ev_timer_init(&timer_repeat_watcher, timer_repeat_cb, 0.5, 2.2);
	ev_timer_start (loop, &timer_repeat_watcher);

	ev_timer_init(&timer_watcher, timer_cb, 5, 0);
	ev_timer_start (loop, &timer_watcher);

	ev_timer_init(&timer_new_watcher, timer_new_cb, 5, 0);
	ev_timer_start (loop, &timer_new_watcher);

	struct ev_signal signal_watcher;
	ev_signal_init(&signal_watcher, signal_cb, SIGINT);
	ev_signal_start(loop, &signal_watcher);

	//如果internal有值, offset为当前的偏移时间
    struct ev_periodic periodic_repeat_watcher;
    //第一次触发在整一分钟内的第30(10+20)s或者下一个20s
    ev_periodic_init(&periodic_repeat_watcher, periodic_repeat_cb, 10, 20, 0);
    ev_periodic_start (loop, &periodic_repeat_watcher);

    //如果internal没值, offset是绝对时间
    struct ev_periodic periodic_watcher;
    ev_periodic_init(&periodic_watcher, periodic_cb, 10000, 0, 0);
    ev_periodic_start (loop, &periodic_watcher);

    //如果reschedule_cb不为空, 就忽略offset和internal
    struct ev_periodic periodic_watcher_resch;
    ev_periodic_init(&periodic_watcher_resch, periodic_resch_cb, 0, 0, scheduler_cb);
    ev_periodic_start (loop, &periodic_watcher_resch);

	ev_run (loop, 0);
}
