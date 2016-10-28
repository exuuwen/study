#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>

#define NCHUNKS 16
#define CHUNK0SIZE 1024

typedef  unsigned int u_int;

struct chunk {
	u_int n_left;
	void *m;
};




struct blocks {
	int id;
	int mark;
	int data;
};

static struct chunk chunks[NCHUNKS];
static int cur_chunk;

void *
newchunk(n)
	u_int n;
{
	struct chunk *cp;
	int k;
	size_t size;

	/* XXX Round up to nearest long. */
	n = (n + sizeof(long) - 1) & ~(sizeof(long) - 1);

	//printf("%s:cur_chunkt:%d\n", __func__, cur_chunk);
	cp = &chunks[cur_chunk];
	if (n > cp->n_left) {
		//printf("%s cp->n_left:%d\n", __func__, cp->n_left);
		++cp, k = ++cur_chunk;
		if (k >= NCHUNKS)
			{
				printf("out of memory\n");
				exit(1);			
			}
		size = CHUNK0SIZE << k;
		cp->m = (void *)malloc(size);
		if (cp->m == NULL)
			{
				printf("out of memory\n");
				exit(1);			
			}
		memset((char *)cp->m, 0, size);
		cp->n_left = size;
		if (n > size)
			{ 
				printf("out of memory\n");
				exit(1);			
			}
	}
	//printf("%s:cur_chunkt:%d\n", __func__, cur_chunk);
	cp->n_left -= n;
	return (void *)((char *)cp->m + cp->n_left);
}


void
freechunks()
{
	int i;

	cur_chunk = 0;
	for (i = 0; i < NCHUNKS; ++i)
		if (chunks[i].m != NULL) {
			//printf("%s:i:%d\n", __func__, i);
			free(chunks[i].m);
			chunks[i].m = NULL;
		}
}



int test()
{
	struct blocks *b;
	
	b = (struct blocks *)newchunk(sizeof(*b));
	freechunks();
	return 0;
}

