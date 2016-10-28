#include <stdio.h> 
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>   
   
int main(int argc, char **argv)
{
	struct in_addr *hipaddr = malloc(sizeof(*hipaddr));
	struct hostent *hptr;
    char *ip = argv[1];
	char **pptr;
	char str[32];
	
	if (argc <= 1)
	{
		printf("%s ip_addr\n", argv[0]);
		return -1;
	}

	/*IP address can be any one that the running host can access*/
	if (!inet_aton(argv[1], hipaddr))
 	{
   		printf("inet_aton error\n");
   		return -1;
 	}

	if ((hptr = gethostbyaddr(hipaddr, sizeof(struct in_addr), AF_INET) ) == NULL)
 	{
  		printf("gethostbyaddr error for addr:%s\n", argv[1]);
  		return -1;
	}

 	printf("Host:%s, official hostname:%s\n", argv[1], hptr->h_name);
	
	if ((hptr = gethostbyname(hptr->h_name)) == NULL)
 	{
  		printf("gethostbyaddr error for addr:%s\n", argv[1]);
  		return -1; 
 	}

 	printf("official hostname:%s, host:%s\n", hptr->h_name, inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str)));/*#define h_addr h_addr_list[0]*/

	for (pptr = hptr->h_addr_list, pptr++; *pptr != NULL; pptr++) 
	        printf(" alias:%s\n", inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
	
	free(hipaddr);

	return 0;
}


