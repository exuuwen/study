#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdlib.h>

#define NETLINK_TEST 20
#define MAX_MSG_PAYLOAD 64
#define PORT 1234

typedef struct SockContext
{
		struct sockaddr_nl sa;
		struct nlmsghdr *nh_send;
		struct nlmsghdr *nh_recv;
		size_t len; // of nh
		int fd;
}SockContext_t;

typedef enum
{
	MSG_TYPE_EVENT_ONE = 1,
	MSG_TYPE_EVENT_TWO = 2,
	MSG_TYPE_EVENT_THREE = 3,
	MSG_TYPE_EVENT_FOUR = 4

}EVENT;

typedef enum
{
	MSG = 1,
	DATA = 2
}MSG_TYPE;

typedef struct data_to_kernel
{
	EVENT event;
	char  info[20];
}data_to_kernel_t;

typedef struct data_from_kernel
{
	EVENT event;
	char  info[20];
}data_from_kernel_t;
