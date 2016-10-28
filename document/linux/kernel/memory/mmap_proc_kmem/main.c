#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>


#define PAGE_SHIFT      12
#define PAGE_SIZE       ((1UL) << PAGE_SHIFT)
#define PAGE_MASK 	(~(PAGE_SIZE-1))
int kfd = -1;

void* get_var(unsigned long addr, unsigned long size, unsigned long *offset) {
        *offset = addr & ~(PAGE_MASK);
        unsigned long ptr = addr & PAGE_MASK;
        void *map;
        

        kfd = open("/dev/kmem",O_RDWR);
        if (kfd < 0) {
                perror("open");
                exit(0);
        }
        map = mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,kfd,ptr);
        if (map == MAP_FAILED) {
                perror("mmap");
                exit(-1);
        }
	
        return map + *offset;
}

int main(int argc, char* argv[])
{
        
        
        unsigned long virtual_addr, virtual_size;
        void *map_addr;
        char s[256];
        int fd;
	unsigned long offset;

	if(argc != 2)
        {
                printf("Usage: %s string\n", argv[0]);
                return 0;
        }
        
        /*get the virtual address of allocated memory in kernel*/
        fd = open("/proc/memshare/virtual_addr", O_RDONLY);
        if(fd < 0)
        {
                printf("cannot open file /proc/memshare/virtual_addr\n");
                return 0;
        }
        read(fd, s, sizeof(s));
        sscanf(s, "%lx", &virtual_addr);
        close(fd);

        /*get the size of allocated memory in kernel*/
        fd = open("/proc/memshare/virtual_size", O_RDONLY);
        if(fd < 0)
        {
                printf("cannot open file /proc/memshare/virtual_size\n");
                return 0;
        }
        read(fd, s, sizeof(s));
        sscanf(s, "%lu", &virtual_size);
        close(fd);
        
        printf("vitual_addr=0x%lx, vitual_size=0x%lx\n", virtual_addr, virtual_size);
	map_addr = get_var(virtual_addr, virtual_size, &offset);
	
	printf("after map\n");
        strcpy(map_addr, argv[1]);
	printf("mem content is:%s\n", (char*)map_addr);

        munmap(map_addr - offset, virtual_size);
        close(kfd);

        return 0;
}
