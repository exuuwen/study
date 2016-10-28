#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

int lock_set(short type, int fd)
{
    struct flock lock;
    
    // global file lock
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_whence = SEEK_SET;
    
    // GETLK does not acutally place the lock on the file. This field will
    // return the lock status. 
    lock.l_type = type;
    
    if (fcntl(fd, F_SETLK, &lock) != 0) /*F_SETLKW will not return and block it until it can get the lock*/
    {        
        int ret = fcntl(fd, F_GETLK, &lock);
        if (ret != 0)
        {
            return -1;
        }
        
        if (lock.l_type == F_UNLCK)
        {
            //printf("file is in UNLOCK status\n");
        }
        else if (lock.l_type == F_RDLCK)
        {
            printf("PID %ld is read locking on the file\n", (long)lock.l_pid);
        }
        else if (lock.l_type == F_WRLCK)  
        {
            printf("PID %ld is write locking on the file\n", (long)lock.l_pid);
        }
                  
        return -1;
    }
    
    return 0;   
}

int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf("usage: ./main filename locktype\n");
		return -1;
	}

	int fd = open(argv[1], O_RDWR);
	if(fd < 0)
	{
		perror("file open fail");
		return -1;
	}

	int type = atoi(argv[2]);
	short locktype;
	if(type == 0)
	{
		locktype = F_RDLCK;    //should not set F_UNLCK first, it is always success, but a new process can not unlock another proceess's lock
	}                          //A rd or wr lock only can be unlock by the same process's unlock
	else
	{
		locktype = F_WRLCK;
	}
	
	int ret = lock_set(locktype, fd);
	if(ret < 0)
	{
		perror("lock file fail");
		return -1;
	}
	else
		printf("lockType %d success on file %s\n", atoi(argv[2]), argv[1]);

	sleep(10);
	
	ret = lock_set(F_UNLCK, fd);
	if(ret < 0)
	{
		perror("lock file fail");
		return -1;
	}
	else
		printf("lockType %d success on file %s\n", atoi(argv[2]), argv[1]);

	while(1);	
}
