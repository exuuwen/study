#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/mman.h>
/*must be open CONFIG_DEVKMEM*/
/*must be check CONFIG_STRICT_DEVMEM both in drivers/char/mem.c
arch/x86/mm/pat.c  can not use range_is_allowed return 0*/
/* CONFIG_STRICT_DEVMEM use function range_is_allowed to allow ram<1M and no ram I/O  return 1*/

#define PAGE_SHIFT      12
#define PAGE_SIZE       ((1UL) << PAGE_SHIFT)
#define PAGE_MASK 	(~(PAGE_SIZE-1))
int kfd = -1;

void* get_var (unsigned long addr, unsigned long  *offset) {
        unsigned long ptr = addr & PAGE_MASK;
        void *map;
       
	*offset = addr & ~(PAGE_MASK);
        kfd = open("/dev/kmem",O_RDONLY);
        if (kfd < 0) {
                perror("open");
                exit(0);
        }
        map = mmap(NULL,PAGE_SIZE,PROT_READ,MAP_SHARED,kfd,ptr);
        if (map == MAP_FAILED) {
                perror("mmap");
                exit(-1);
        }
	
        return map + *offset;
}
int main(int argc, char **argv)
{
        FILE *fp;
        char addr_str[11]="0x";
        char var[51];
        unsigned long addr;
	unsigned long offset;
        char ch;
        int r;
	void* map_addr;
        
        if (argc != 3) {
                fprintf(stderr,"usage: %s System.map var\n",argv[0]);
                exit(-1);
        }
        if ((fp = fopen(argv[1],"r")) == NULL) {
                perror("fopen");
                exit(-1);
        }
        do {
                r = fscanf(fp,"%8s %c %50s\n",&addr_str[2],&ch,var);
                if (strcmp(var, argv[2])==0)
                        break;
        } while(r > 0);
        if (r < 0) {
                printf("could not find modprobe_path\n");
                exit(-1);
        }
        addr = strtoul(addr_str,NULL,16);

        printf("found %s at (%s) %08lx\n", argv[2],addr_str,addr);
	
        map_addr = get_var(addr, &offset);

	printf("%s\n", (char*)map_addr);

	munmap(map_addr - offset, PAGE_SIZE);
	close(kfd);
	
}
