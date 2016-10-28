#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/socket.h>
#include <linux/veth.h>

#include "rtnetlink.h"
#include <linux/netlink.h>

int rt_open(struct sockaddr_nl *loc_addr) 
{
    int fd;
    socklen_t alen;

    if ((fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0) 
    {
        printf("Failed to open socket: %s\n", strerror(errno));		
        goto end;
    }

    memset(loc_addr, 0, sizeof(*loc_addr));
    loc_addr->nl_family = AF_NETLINK;
    loc_addr->nl_groups = 0; 
	
    if (bind(fd, (struct sockaddr*)loc_addr, sizeof(*loc_addr)) < 0) 
    {
        printf("Failed to bind to socket: %s\n", strerror(errno));
        close(fd);
        fd = -1;
        goto end;
    }

    alen = sizeof(*loc_addr);
    if ((getsockname(fd, (struct sockaddr *)loc_addr, &alen) < 0) && 
        (alen != sizeof(*loc_addr) || loc_addr->nl_family != AF_NETLINK))
    {
        printf("Failed getsockname() or unexpected results:\n"
            "errno: %s\taddress family: %d\taddress length: %d\n", 
                strerror(errno), loc_addr->nl_family, alen);
        close(fd);		
        fd = -1;
        goto end;
    }
 
end:
    return fd;
}

#define NLBUFSIZE 8192

int rt_find_ack(struct nlmsghdr *hdr, 
		   struct sockaddr_nl *nladdr, 
		   struct msghdr *nlmsg, 
		   struct sockaddr_nl *loc_addr,		   
		   unsigned int *seq, int r) 
{
    while (r >= (int) sizeof(*hdr)) 
    {
        int remaining;

        remaining = hdr->nlmsg_len - sizeof(*hdr); /* What remains beyond the header */
		
        /* Check that we got all of the message */
        if (remaining < 0 || (int) hdr->nlmsg_len > r) 
        {
            if (nlmsg->msg_flags & MSG_TRUNC) 
            {
                printf("Received netlink message truncated\n");
                r =  -1;
                goto end;
            }

            printf("Received netlink message length incorrect: %d\n", hdr->nlmsg_len);
            r = -1;
            goto end;
        }
		
        /* Ensure the message is for us */
        if (nladdr->nl_pid != 0 ||                    /* Valid process ID */
             hdr->nlmsg_seq != *seq ||       /* Is reply for our request? */
                 hdr->nlmsg_pid != loc_addr->nl_pid) /* Is for this process */ 
        {
            continue;                /* Otherwise ignore message */ 
        }
		
        /* ACK messages are of type NLMSG_ERROR with errno 0 */
        if (hdr->nlmsg_type == NLMSG_ERROR) 
        {
            struct nlmsgerr *error = (struct nlmsgerr*)NLMSG_DATA(hdr);
            if (remaining >= (int) sizeof(*error)) 
            {
                if ((errno = -error->error) == 0) 
                {
                    /* Got an ACK, we're done here */
                    r = 0;
                    goto end;
                }

                printf("Netlink: %d, %s\n", errno, strerror(errno));
            } 
            else 
                printf("Error message truncated\n");

            r = -1;
            goto end;
        }

        printf("Netlink message type NLMSG_ERROR(%d) expected but got %d???\n", NLMSG_ERROR, hdr->nlmsg_type);
        r -= NLMSG_ALIGN(hdr->nlmsg_len);

        hdr = (struct nlmsghdr*)((char*)hdr + NLMSG_ALIGN(hdr->nlmsg_len));
    }

end:
    return r;
}

int rt_send(int fd, unsigned int *seq, struct nlmsghdr *n, struct sockaddr_nl *loc_addr)
{
    struct sockaddr_nl nladdr;
    struct iovec iov;
    struct msghdr nlmsg;
    int r = 0;
    char* buffer = NULL;
	
    buffer = malloc(NLBUFSIZE);
    assert(buffer);
	
    iov.iov_base = (void *)n;
    iov.iov_len = n->nlmsg_len;
	
    memset(&nlmsg, 0, sizeof(nlmsg));
    nlmsg.msg_name = (void *)&nladdr;
    nlmsg.msg_namelen = sizeof(nladdr);
    nlmsg.msg_iov = &iov;
    nlmsg.msg_iovlen = 1; 

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    n->nlmsg_seq = ++(*seq);
    n->nlmsg_flags |= NLM_F_ACK;
	
    r = sendmsg(fd, &nlmsg, 0);

    if (r < 0) 
    {
        printf("sendmsg() failed: %s\n", strerror(errno));		
        goto end;
    }

    iov.iov_base = buffer;

    while (1) 
    {	
        iov.iov_len = NLBUFSIZE;
        r = recvmsg(fd, &nlmsg, 0);
		
        if (r < 0) 
        {
            if (errno != EINTR) /* EINTR is "normal", don't warn */
                 printf("recvmsg() failed: %s\n", strerror(errno));		
            continue;
        }
		
        if (r == 0) 
        {
            printf("EOF on netlink socket\n");
            r = -1;
            goto end;
        }

        r = rt_find_ack((struct nlmsghdr*)buffer, &nladdr, &nlmsg, loc_addr, seq, r);
        if (r <= 0) 
            goto end;		       

        if (nlmsg.msg_flags & MSG_TRUNC) 
        {
            printf("Got truncated message!\n");
            r = -1;
            goto end;
        }

        if (r) 
        {
            printf("Insufficient data left for header!\n");
            r = -1;
            goto end;			
        }
    }

end:
    if (buffer) 
    {
        free(buffer);
    }

    return r;
}


struct rtattr* rt_addAttr_data(struct nlmsghdr *n, int type, const void *data, int len)
{ 
	struct rtattr *rta;
	int rlen = RTA_LENGTH(len);

	rta = (struct rtattr*)(((char*)n) + NLMSG_ALIGN(n->nlmsg_len));
	rta->rta_type = type;
	rta->rta_len = rlen; 

	assert(data != NULL);
	memcpy(RTA_DATA(rta), data, len);

	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + rlen;

	return rta;
}

struct rtattr * rt_addAttr_hdr(struct nlmsghdr *n, int type)
{
	struct rtattr *rta;
	int rlen = RTA_LENGTH(0); 

	rta = (struct rtattr*)(((char*)n) + NLMSG_ALIGN(n->nlmsg_len));
	rta->rta_type = type;
	rta->rta_len = rlen; 

	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + rlen;

	return rta;
}

void rt_compAttr_hdr(struct nlmsghdr *n, struct rtattr * rta_head)
{
	rta_head->rta_len = (char*)n + n->nlmsg_len - (char*)rta_head;

	return;
}


void rt_link_init(struct nlmsghdr *n, int cmd, unsigned short flags)
{
	n->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
	n->nlmsg_type = cmd; /* RTM_NEWLINK or RTM_DELLINK */
	n->nlmsg_flags = flags;
}

void rt_addr_init(struct nlmsghdr *n, int cmd, unsigned short flags)
{
	n->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	n->nlmsg_type = cmd; 
	n->nlmsg_flags = flags;
}

void rt_route_init(struct nlmsghdr *n, int cmd, unsigned short flags)
{
	n->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	n->nlmsg_type = cmd;
	n->nlmsg_flags = flags;
}

void rt_neigh_init(struct nlmsghdr *n, int cmd, unsigned short flags)
{
	n->nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg));
	n->nlmsg_type = cmd;
	n->nlmsg_flags = flags;
}


