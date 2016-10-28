#include "test_netlink.h"

static SockContext_t ctx;
static int pfd[2] ;
static int stopping = 1;

static int recv_data()
{
	int retval = 0;
	data_from_kernel_t *datab = NULL;
	struct sockaddr_nl *sa = &ctx.sa;
	int fd = ctx.fd;
	struct nlmsghdr *nh = ctx.nh_recv;
	assert(nh);


	nh->nlmsg_len = NLMSG_SPACE(sizeof(data_from_kernel_t));
	nh->nlmsg_flags |= NLM_F_ACK;

	struct iovec iov = { (void *) nh, nh->nlmsg_len };

	memset(sa, 0 ,sizeof(*sa));
	sa->nl_family= AF_NETLINK ;
	struct msghdr msg = { (void *)sa, sizeof(*sa), &iov, 1, NULL, 0, 0 };
	retval = recvmsg(fd, &msg, 0);
	assert(retval > 0);
	if (retval < 0) {
		printf("recvmsg error\n");
		perror("recvmsg");
	}
	printf("received msg len: %d\n", nh->nlmsg_len);

	unsigned short size = (nh->nlmsg_len - sizeof(*nh));
	datab = NLMSG_DATA(nh);
	//printf("received payload len: %d data type is %d ,data is %s\n", size, datab->event, datab->info);
	printf("datab->DstPort is:%d\n",datab->DstPort);
	printf("datab->SrcIp is:%d\n",datab->SrcIp);
	
}
/*data_to_kernel_t events = { MSG_TYPE_EVENT_ONE, "data_to_kernel"};

static int send_data()
{
	int ret = 0;
	data_to_kernel_t *datab = NULL;
	struct sockaddr_nl *sa = &ctx.sa;
	int fd = ctx.fd;
	struct nlmsghdr *nh = ctx.nh_send;
	assert(nh);
	nh->nlmsg_len = NLMSG_SPACE(sizeof(data_to_kernel_t));
	nh->nlmsg_flags |= NLM_F_ACK;
	nh->nlmsg_type = DATA;

	datab = NLMSG_DATA(nh);
	///////////////do yourself data///////////////////////////
	memcpy(datab, &events, sizeof(events));
	///////////////////////////////////////////

	struct iovec iov = { (void *) nh, nh->nlmsg_len };

	memset(sa, 0 ,sizeof(*sa));

	sa->nl_family= AF_NETLINK ;

	struct msghdr msg = { (void *)sa, sizeof(*sa), &iov, 1, NULL, 0, 0 };

	ret = sendmsg(fd, &msg, 0);
	if (ret < 0)
	{
		perror("sendmsg");
		assert(0);
	}
	return ret;
}*/


int start_netlink()
{
	/*struct sockaddr_nl *sa = &ctx.sa;
	int fd = ctx.fd;
	unsigned char *datab = NULL;
	struct nlmsghdr *nh = ctx.nh_send;*/
	int fd = ctx.fd;
	fd_set rfds;
	int retval;

	//assert(nh);

	while (!stopping)
	{
		/*send_data();
		printf("waiting for msg from kernel...\n");*/

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		FD_SET(pfd[0], &rfds);

		retval = select(fd > pfd[0] ? (fd + 1) : (pfd[0] + 1), &rfds, NULL, NULL, NULL);
		switch (retval) {
		case EBADF:
			assert(0);
			break;

		case EINTR:
			printf("select interrupted\n");
			continue;
			break;

		default:
			if (FD_ISSET(pfd[0], &rfds)) {
				printf("wakeup call, time to exit thread\n");
				assert(stopping);
				continue;
			}
			break;
		}

		assert(FD_ISSET(fd, &rfds));

		recv_data();
	}
	return 0;
}


static int send_msg()
{
	int ret = 0;

	struct sockaddr_nl *sa = &ctx.sa;
	int fd = ctx.fd;
	struct nlmsghdr *nh = ctx.nh_send;
	assert(nh);
	nh->nlmsg_len = NLMSG_SPACE(0);
	nh->nlmsg_pid = getpid();
	nh->nlmsg_flags |= NLM_F_ACK;
	nh->nlmsg_type = MSG;
	struct iovec iov = { (void *) nh, nh->nlmsg_len };

	memset(sa, 0 ,sizeof(*sa));

	sa->nl_family= AF_NETLINK ;

	struct msghdr msg = { (void *)sa, sizeof(*sa), &iov, 1, NULL, 0, 0 };
	printf("before send message\n");
	ret = sendmsg(fd, &msg, 0);
	printf("ret(%d)after send message\n", ret);
	if (ret < 0)
	{
		perror("sendmsg");
		assert(0);
	}
	return ret;
}

int  init_netlink()
{
	int ret = 0;

	memset(&ctx, 0, sizeof(ctx));

	ctx.fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
	assert(ctx.fd != -1);

	struct sockaddr_nl *sa = &ctx.sa;
	sa->nl_family= AF_NETLINK;
	sa->nl_pid = getpid();

	ret = bind(ctx.fd, (struct sockaddr*)sa, sizeof(*sa));
	assert(ret == 0);

	ctx.len = NLMSG_SPACE(MAX_MSG_PAYLOAD);
	ctx.nh_recv = (struct nlmsghdr *)malloc(ctx.len);
	ctx.nh_send = (struct nlmsghdr *)malloc(ctx.len);
	assert(ctx.nh_recv);
	assert(ctx.nh_send);
	stopping = 0;

	return ret;
}
int main(int argc, char **argv)
{
	int ret = 0;

	ret = init_netlink();
	send_msg();
	start_netlink();

	return 0;
}
