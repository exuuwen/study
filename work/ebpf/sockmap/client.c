#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>

#include <linux/netlink.h>
#include <linux/socket.h>
#include <linux/if_link.h>
#include <assert.h>
#include <libgen.h>

#define S1_PORT 10000

int running;
void running_handler(int a);

static int sockmap_test_sockets(int rate, int dot)
{
	int i, sc, err, one = 1;
	struct sockaddr_in addr;
	char buf[1024] = {0};
	int fd;

	/* Init sockets */
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket s1 failed()");
		err = fd;
		goto out;
	}


	/* Bind server sockets */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	/* Initiate Connect */
	addr.sin_port = htons(S1_PORT);
	err = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
	if (err < 0 && errno != EINPROGRESS) {
		perror("connect fd failed()\n");
		goto out;
	}

	unsigned char data = 0;
	buf[0] = data;


	/* Ping/Pong data from client to server */
	sc = send(fd, buf, sizeof(buf), 0);
	if (sc < 0) {
		perror("send failed()\n");
		goto out;
	}

	do {
		int rc;
		//printf("before recve\n");

		rc = recv(fd, buf, sizeof(buf), 0);
		if (rc < 0) {
			if (errno != EWOULDBLOCK) {
				perror("recv failed()\n");
				break;
			}
		}
		printf("buf[0] %d\n", buf[0]);

		if (rc == 0) {
			break;
		}

		buf[0] += 1;
		sc = send(fd, buf, rc, 0);
		if (sc < 0) {
			perror("send failed()\n");
			break;
		}
		sleep(rate);
		if (dot) {
			printf(".");
			fflush(stdout);

		}
	} while (running);

out:
	close(fd);
	return err;
}

int main(int argc, char **argv)
{
	int rate = 1, dot = 1;
	running = 1;

	/* catch SIGINT */
	signal(SIGINT, running_handler);

	int err = sockmap_test_sockets(rate, dot);
	if (err) {
		fprintf(stderr, "ERROR: test socket failed: %d\n", err);
		return err;
	}
	return 0;
}

void running_handler(int a)
{
	running = 0;
}

