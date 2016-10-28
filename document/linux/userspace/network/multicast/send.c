#include <stdio.h>
#include <sys/prctl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sched.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#define MCAST_ADDR "224.0.0.88"    
#define MCAST_DATA "MULTICAST TEST DATA"            
#define MCAST_INTERVAL 2

int main(int argc, char*argv[])
{
	int s, n, err = 0;
	struct sockaddr_in mcast_addr;
	struct in_addr localInterface;
     
	if (argc != 3)
	{
		printf("%s local_ip remote_port\n", argv[0]);
		return -1;
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);         
	if (s == -1)
	{
		perror("socket()");
		return -1;
	}
	
	memset(&mcast_addr, 0, sizeof(mcast_addr));
	mcast_addr.sin_family = AF_INET;                
	mcast_addr.sin_addr.s_addr = inet_addr(MCAST_ADDR);
	mcast_addr.sin_port = htons(atoi(argv[2]));

	/*struct ifreq interface;
	strncpy(interface.ifr_ifrn.ifrn_name, "eth0", 5);
	err = setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, (char *)&interface, sizeof(interface));
	if(err < 0)
	{
		perror("setsockopt():bind device");
		return -1;
	}*/

	/*send the muticast packet through the specfic interface, and set the src of packet as the interface*/
	localInterface.s_addr = inet_addr(argv[1]);
	if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, (char *) &localInterface, sizeof(localInterface)) < 0) 
	{
		perror("setsockopt mulicast if");
		return -1;
	}
	
	/*allow the multicast packet is recieved by local reciever, default is enable*/
	int loop = 1;
	err = setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
	if(err < 0)
	{
		perror("setsockopt():IP_MULTICAST_LOOP");
		return -3;
	}

	while(1) 
	{
		n = sendto(s, MCAST_DATA, sizeof(MCAST_DATA), 0, (struct sockaddr*)&mcast_addr, sizeof(mcast_addr)) ;
		if( n < 0)
		{
		    perror("sendto()");
		    return -2;
		}      

		sleep(MCAST_INTERVAL);                          
	}

	return 0;
}
