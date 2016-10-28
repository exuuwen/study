#include <stdio.h>
#include <string.h>
#include <ev.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>

#define EVENT_SIZE  (sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

static int inotify_fd;
static int watch_fd;

static struct ev_loop * loop;
static struct ev_io inotify_watcher;
static struct ev_io cmdline_watcher;
static char* dir = "/root/watchdir/";

static void
inotify_cb (__attribute__((unused)) struct ev_loop *loop, ev_io *w, int revents)
{
    if ((revents & EV_READ) && (w->fd == inotify_fd))
    {
        int length, j = 0;
        char buffer[EVENT_BUF_LEN];
        length = read(w->fd, buffer, EVENT_BUF_LEN);
        if (length < 0)
            return;

        while (j < length)
        {
            struct inotify_event *event = (struct inotify_event*)&buffer[j];
			
			if (event->len)
			{ 
            	if ( event->mask & IN_CLOSE_WRITE)
            	{
                	printf("inclose hahahaha %s, %d\n", event->name, event->len);
            	}
				else if ( event->mask & IN_MOVED_TO)
            	{
                	printf("move to hahahaha %s, %d\n", event->name, event->len);
            	}
			}

            j += EVENT_SIZE + event->len;
        }
    }
}

int main()
{
    loop = ev_default_loop(0);
    if (loop == NULL)
        return -1;

    inotify_fd = inotify_init1(IN_NONBLOCK);
    if (inotify_fd < -1)
        return -1;

    watch_fd = inotify_add_watch(inotify_fd, dir, IN_CLOSE_WRITE | IN_MOVED_TO);
    if (watch_fd < 0)
    {
        close(inotify_fd);
        return -1;
    }

    ev_init(&inotify_watcher, inotify_cb);
    ev_io_set(&inotify_watcher, inotify_fd, EV_READ);
    ev_io_start(loop, &inotify_watcher);

	ev_run(loop, 0);

    return 0;
}
