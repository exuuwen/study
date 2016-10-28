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
#include <sys/capability.h>
#include <sys/socket.h>
#include <netinet/in.h>

int set_capabilities(cap_value_t cap_list[], int ncap) 
{
	cap_t caps;
	int ret;

	/* caps should be initialized with all flags cleared... */
	caps = cap_init();
	if (!caps) 
	{
		perror("cap_init");
		return -1;
	}
	/* ... but we better rely on cap_clear */
	if (cap_clear(caps)) 
	{
		perror("cap_clear");
		return -1;
	}

	if ((cap_list) && ncap) 
	{
		if (cap_set_flag(caps, CAP_PERMITTED, ncap, cap_list, CAP_SET) != 0) 
		{
			perror("cap_set_flag CAP_PERMITTED");
			cap_free(caps);
			return -1;
		}
		if (cap_set_flag(caps, CAP_EFFECTIVE, ncap, cap_list, CAP_SET) != 0)
		{
			perror("cap_set_flag CAP_EFFECTIVE");
			cap_free(caps);
			return -1;
		}
		if (cap_set_flag(caps, CAP_INHERITABLE, ncap, cap_list, CAP_SET) != 0)
		{
			perror("cap_set_flag CAP_INHERITABLE");
			cap_free(caps);
			return -1;
		}
		
	}

	ret = cap_set_proc(caps);

	if (ret) 
	{
		perror("cap_set_proc");
		cap_free(caps);
		return -1;
	}

	if (cap_free(caps)) 
	{
		perror("cap_free");
		return -1;
	}

	return 0;
}

int drop_capabilities(cap_value_t cap_list[], int ncap) 
{
	cap_t caps;
	int ret;

	/* caps should be initialized with all flags cleared... */
	caps = cap_get_proc();
	if (!caps) 
	{
		perror("cap_init");
		return -1;
	}

	if ((cap_list) && ncap) 
	{
		if (cap_set_flag(caps, CAP_EFFECTIVE, ncap, cap_list, CAP_CLEAR) != 0) 
		{
			perror("cap_set_flag CAP_PERMITTED");
			cap_free(caps);
			return -1;
		}
		if (cap_set_flag(caps, CAP_PERMITTED, ncap, cap_list, CAP_CLEAR) != 0)
		{
			perror("cap_set_flag CAP_EFFECTIVE");
			cap_free(caps);
			return -1;
		}
		if (cap_set_flag(caps, CAP_INHERITABLE, ncap, cap_list, CAP_CLEAR) != 0)
		{
			perror("cap_set_flag CAP_INHERITABLE");
			cap_free(caps);
			return -1;
		}
	}

	ret = cap_set_proc(caps);

	if (ret) 
	{
		perror("cap_set_proc");
		cap_free(caps);
		return -1;
	}

	return 0;
}

int main()
{
	
	int ch, ret = -1;
	cap_value_t cap_list[4];
	cap_t caps;
	cap_value_t drop_cap[2];

	if (geteuid()) 
	{
		fprintf(stderr, "The sandbox is not seteuid root, aborting\n");
		return EXIT_FAILURE;
	}

	if (!getuid()) 
	{
		fprintf(stderr, "The sandbox is not designed to be run by root, aborting\n");
		return EXIT_FAILURE;
	}

	/* capabilities we need */
	cap_list[0] = CAP_SETUID;
	cap_list[1] = CAP_SETGID;
	cap_list[2] = CAP_SYS_ADMIN;  /*for clone*/
	cap_list[3] = CAP_NET_RAW;

	//set the cap we need
	if (set_capabilities(cap_list, sizeof(cap_list)/sizeof(cap_list[0]))) 
	{
		fprintf(stderr, "Could not adjust capabilities, aborting\n");
		return EXIT_FAILURE;
	}

	ret = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	assert(ret != -1);
	printf("ok, run raw socket sucessful\n");

	caps = cap_get_proc();
	if(!caps)
	{
		perror("get proc");
		return EXIT_FAILURE;
	}

	printf("test_cap: running with caps %s\n", cap_to_text(caps, NULL));

	drop_cap[0] = CAP_SETUID;
	drop_cap[1] = CAP_NET_RAW;

	// drop the caps above
	if (drop_capabilities(drop_cap, sizeof(drop_cap)/sizeof(drop_cap[0])))
	{
		fprintf(stderr, "Could not adjust capabilities, aborting\n");
		return EXIT_FAILURE;
	}
	
	caps = cap_get_proc();
	if(!caps)
	{
		perror("get proc");
		return EXIT_FAILURE;
	}

	printf("test_cap: after drop running with caps %s\n", cap_to_text(caps, NULL));
	
	// must be can not raw socket ecause of the drop CAP_NET_RAW
	ret = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	assert(ret == -1);

	printf("ok, run raw socket failed\n");

	//drop all the caps
	if (set_capabilities(NULL, 0))
	{
		fprintf(stderr, "Could not adjust capabilities, aborting\n");
		return EXIT_FAILURE;
	}
	
	caps = cap_get_proc();
	if(!caps)
	{
		perror("get proc");
		return EXIT_FAILURE;
	}

	printf("test_cap: after drop all running with caps %s\n", cap_to_text(caps, NULL));
	

 	return 0;

}
