#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
/*
$ ls -l tempfile            look at how big the file is
    -rw-r----- 1 sar     413265408 Jan 21 07:14 tempfile
    $ df /home                  check how much free space is available
    Filesystem  1K-blocks     Used  Available  Use%  Mounted  on
    /dev/hda4    11021440  1956332    9065108   18%  /home
    $ ./a.out &                 run the program in Figure 4.16 in the background
    1364                        the shell prints its process ID
    $ file unlinked             the file is unlinked
    ls -l tempfile              see if the filename is still there
    ls: tempfile: No such file or directory           the directory entry is gone
    $ df /home                  see if the space is available yet
    Filesystem  1K-blocks     Used  Available  Use%  Mounted  on
    /dev/hda4    11021440  1956332    9065108   18%  /home
    $ done                      the program is done, all open files are closed
    df /home                    now the disk space should be available
    Filesystem  1K-blocks     Used  Available  Use%  Mounted on
    /dev/hda4    11021440  1552352    9469088   15%  /home
*/
/*when unlink and the link num = 0, then delete the stuff on the file system*/
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

	ret = unlink(argv[1]);
        assert(ret == 0);
    	printf("file unlinked\n");
    
	sleep(20);

	close(fd);
    
	printf("done\n");
	
	return 0;
}
