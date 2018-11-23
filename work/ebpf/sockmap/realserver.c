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

#define S2_PORT 10001

int running;
void running_handler(int a);

static int sockmap_test_sockets(int rate, int dot)
{
	int i, sc, err, one = 1;
	struct sockaddr_in addr;
	char buf[1024] = {0};
	int fd;
	int sfd;


	/* Init sockets */
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd < 0) {
		perror("socket sfd failed()");
		err = sfd;
		goto out;
	}

	err = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
			 (char *)&one, sizeof(one));
	if (err) {
		perror("setsockopt failed()");
		goto out;
	}

	/* Bind server sockets */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	addr.sin_port = htons(S2_PORT);
	err = bind(sfd, (struct sockaddr *)&addr, sizeof(addr));
	if (err < 0) {
		perror("bind realserver failed()\n");
		goto out;
	}

	addr.sin_port = htons(S2_PORT);
	err = listen(sfd, 32);
	if (err < 0) {
		perror("listen realserver failed()\n");
		goto out;
	}

	fd = accept(sfd, NULL, NULL);
	if (fd < 0) {
		perror("accept sfd failed()\n");
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
		buf[0] += 1;

		if (rc == 0) {
			close(fd);
			break;
		}

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

