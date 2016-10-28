#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <net/ethernet.h>
#include <poll.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h> 
#include <signal.h> 
#include <stdio.h> 
#include <string.h> 
#include <sys/mman.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

static void
handler(int sig, siginfo_t *si, void *unused)
{
    printf("Got SIGSEGV at address: 0x%lx\n",
            (long) si->si_addr);
    exit(EXIT_FAILURE);
}

static inline unsigned char *emit_code(unsigned char *ptr, unsigned int bytes, unsigned int len)
{
	if (len == 1)
		*ptr = bytes;
	else if (len == 2)
		*(unsigned short *)ptr = bytes;
	else {
		*(unsigned int *)ptr = bytes;
		//barrier();
	}
	return ptr + len;
} 
#define EMIT(bytes, len)	do { prog = emit_code(prog, bytes, len); } while (0)
#define EMIT1(b1)		EMIT(b1, 1)
#define EMIT2(b1, b2)		EMIT((b1) + ((b2) << 8), 2)
#define EMIT3(b1, b2, b3)	EMIT((b1) + ((b2) << 8) + ((b3) << 16), 3)
#define EMIT4(b1, b2, b3, b4)   EMIT((b1) + ((b2) << 8) + ((b3) << 16) + ((b4) << 24), 4)
struct _handle 
{
 unsigned int  (*bpf_func)();
}; 
static struct _handle handle;
unsigned char *image = NULL;
int alloc_size = 0;
static int get_image() 
{
	
	unsigned char temp[64];
	unsigned char *prog; 
	unsigned char proglen = 6; 
	prog = temp; 
	EMIT1(0xb8);
	EMIT4(0x15,00,00,00);  //movl $21, %eax
	EMIT1(0xc3);  //ret  
	
	alloc_size = getpagesize(); 
	int fd = open ("/dev/zero", O_RDONLY); 
	image = mmap (NULL, alloc_size, PROT_NONE , MAP_PRIVATE, fd, 0); 
	if(image == MAP_FAILED)
	{
		printf("map erron\n");
		return -1;
	} 
	mprotect (image, alloc_size, PROT_WRITE);
	memcpy(image, temp, proglen);
        
	int i;
	for(i=0; i<proglen; i++)
	{
		printf("0x%x ", image[i]);
	}
	printf("\n");
	mprotect (image, alloc_size, PROT_EXEC);
	
	printf("image:%p\n", image);
	handle.bpf_func = (void *)image; 
	return   0; 
} 

int main(void) 
{
	int ret = 0;	
	 struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = handler;
	if (sigaction(SIGSEGV, &sa, NULL) == -1)
		handle_error("sigaction");

	ret = get_image();
	printf("get_image() ret:%d\n", ret); 
	if(!ret) 		
	{
		ret = handle.bpf_func(); 
		printf(" handle.bpf_func() ret:%d\n",ret); 		
	}

	munmap(image, alloc_size);
    
	return 0; 
}


