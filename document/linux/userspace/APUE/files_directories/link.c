#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
/*
cr7@cr7-virtual-machine:~/test/APUE/files_directories$ df /home/cr7/test/APUE/files_directories/
Filesystem           1K-blocks      Used Available Use% Mounted on
/dev/sda1             39748204   9637228  28091856  26% /
cr7@cr7-virtual-machine:~/test/APUE/files_directories$ ./unlink  unlink.txt &
[1] 14938
cr7@cr7-virtual-machine:~/test/APUE/files_directories$ file linked

cr7@cr7-virtual-machine:~/test/APUE/files_directories$ df /home/cr7/test/APUE/files_directories/
Filesystem           1K-blocks      Used Available Use% Mounted on
/dev/sda1             39748204   9637228  28091856  26% /
cr7@cr7-virtual-machine:~/test/APUE/files_directories$ file unlinked

cr7@cr7-virtual-machine:~/test/APUE/files_directories$ df /home/cr7/test/APUE/files_directories/
Filesystem           1K-blocks      Used Available Use% Mounted on
/dev/sda1             39748204   9637232  28091852  26% /
cr7@cr7-virtual-machine:~/test/APUE/files_directories$ done

[1]+  Done                    ./unlink unlink.txt
cr7@cr7-virtual-machine:~/test/APUE/files_directories$ df /home/cr7/test/APUE/files_directories/
Filesystem           1K-blocks      Used Available Use% Mounted on
/dev/sda1             39748204   9637232  28091852  26% /
*/
/*link do not create new stuff in filte system, just 
share the old one and link num ++, when unlink and 
the link num = 0, then delete the stuff on the file system*/

int main(int argc, char *argv[])
{
	int ret;
	int fd;
	int len;

	if (argc != 2)
	{
		printf("usage: a.out <pathname> ");
		assert(argc == 2);	
	}

	fd = open(argv[1], O_RDWR);
	assert(fd >= 0);
	
	ret = link(argv[1], "link.txt");
	assert(ret == 0);
	printf("file linked\n");

	sleep(20);

	ret = unlink(argv[1]);
        assert(ret == 0);
    	printf("file unlinked\n");
    
	sleep(20);

	close(fd);
    
	printf("done\n");
	
	return 0;
}
