#include <stdio.h>
#include <string.h>
#include <stdlib.h> 

#include "gencode.h"

#define FILTER_LEN 1024
int main(int argc, char **argv)
{
	struct bpf_program code;
	int i;
	char *filter_buf = (char*)malloc(FILTER_LEN); 
	int  arg_begin = 1;
	int offset = 0;
	
	if(argc > 1){
		for(i=arg_begin; i<argc; i++)
         	{   
              		strcpy(filter_buf + offset,argv[i]);
              		offset=offset + strlen(argv[i]) + 1;
              		memset(filter_buf + offset -1 , ' ', 1);                
         	}
		bfp_compile(filter_buf, &code);	
		printf("code len is %d\n", code.bf_len);
		bpf_dump(&code, 3);
	}	
	free(filter_buf);
	
	return 0;
}
