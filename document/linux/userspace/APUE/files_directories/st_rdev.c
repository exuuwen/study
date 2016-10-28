#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
cr7@cr7-virtual-machine:~/APUE/4$ ./st_rdev /dev/tty[0123]  /dev/sda1 /home/
/dev/tty0: dev = 0/5 (character) rdev = 4/0
/dev/tty1: dev = 0/5 (character) rdev = 4/1
/dev/tty2: dev = 0/5 (character) rdev = 4/2
/dev/tty3: dev = 0/5 (character) rdev = 4/3
/dev/sda1: dev = 0/5 (block) rdev = 8/1
/home/: dev = 8/1
*/
/*Only character special files and block special files have an st_rdev value*/
int main(int argc, char *argv[])
{
	int i;
	struct stat buf;

	assert(argc > 1);

	for (i = 1; i < argc; i++) 
	{
		printf("%s: ", argv[i]);
		if (stat(argv[i], &buf) < 0) 
		{
			perror("stat error");
			continue;
		}

		printf("dev = %d/%d", major(buf.st_dev), minor(buf.st_dev));
		if (S_ISCHR(buf.st_mode) || S_ISBLK(buf.st_mode)) 
		{
			printf(" (%s) rdev = %d/%d",
			     (S_ISCHR(buf.st_mode)) ? "character" : "block",
			     major(buf.st_rdev), minor(buf.st_rdev));

		}
		printf("\n");
	}

	return 0;

}



