#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

static inline uint64_t
rand64(void)
{
	uint64_t val;
	val = lrand48();
	val <<= 32;
	val += lrand48();
	return val;
}

int main(int argc, char *argv[])
{
    int j, r, nloops;
    unsigned long rl;
    unsigned int seed;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <seed> <nloops>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    seed = atoi(argv[1]);
    nloops = atoi(argv[2]);

    srand(seed);
    for (j = 0; j < nloops; j++) {
        r =  rand();
        printf("%d\n", r);
    }

    srand48(seed);
    for (j = 0; j < nloops; j++) {
        rl = rand64();
        printf("%lu\n", rl);
    }

    exit(EXIT_SUCCESS);
}
