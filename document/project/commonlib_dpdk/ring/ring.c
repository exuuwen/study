#include "ring.h"


/* true if x is a power of 2 */
#define POWEROF2(x) ((((x)-1) & (x)) == 0)

/*delete the ring*/
void lib_ring_destruct(struct lib_ring* r)
{
	free(r);
}

/* create the ring */
struct lib_ring *
lib_ring_create(const char *name, unsigned count,
		unsigned flags)
{
	//char mz_name[MEMZONE_NAMESIZE];
	struct lib_ring *r;
	//const struct rte_memzone *mz;
	size_t ring_size;

	/* count must be a power of 2 */
	if (!POWEROF2(count)) {
		perror("Requested size is not a power of 2\n");
		return NULL;
	}

	//snprintf(mz_name, sizeof(mz_name), "RG_%s", name);
	ring_size = count * sizeof(void *) + sizeof(struct lib_ring);

	/* reserve a memory zone for this ring */
	r = (struct lib_ring *)malloc(ring_size);
	if (r == NULL) {
		perror("Cannot malloc reserve memory\n");
		return NULL;
	}

	//r = mz->addr;

	/* init the ring structure */
	memset(r, 0, sizeof(*r));
	snprintf(r->name, sizeof(r->name), "%s", name);
	r->flags = flags;
	r->prod.bulk_default = r->cons.bulk_default = 1;
	r->prod.watermark = count;
	r->prod.sp_enqueue = !!(flags & RING_F_SP_ENQ);
	r->cons.sc_dequeue = !!(flags & RING_F_SC_DEQ);
	r->prod.size = r->cons.size = count;
	r->prod.mask = r->cons.mask = count-1;
	r->prod.head = r->cons.head = 0;
	r->prod.tail = r->cons.tail = 0;

	return r;
}

/*
 * change the high water mark. If *count* is 0, water marking is
 * disabled
 */
int
lib_ring_set_water_mark(struct lib_ring *r, unsigned count)
{
	if (count >= r->prod.size)
		return -EINVAL;

	/* if count is 0, disable the watermarking */
	if (count == 0)
		count = r->prod.size;

	r->prod.watermark = count;
	return 0;
}

/* dump the status of the ring on the console */
void
lib_ring_dump(const struct lib_ring *r)
{
#ifdef RTE_LIBRTE_RING_DEBUG
	struct rte_ring_debug_stats sum;
#endif

	printf("ring <%s>@%p\n", r->name, r);
	printf("  flags=%x\n", r->flags);
	printf("  size=%d\n", r->prod.size);
	printf("  ct=%d\n", r->cons.tail);
	printf("  ch=%d\n", r->cons.head);
	printf("  pt=%d\n", r->prod.tail);
	printf("  ph=%d\n", r->prod.head);
	printf("  used=%d\n", ring_count(r));
	printf("  avail=%d\n", ring_free_count(r));
	if (r->prod.watermark == r->prod.size)
		printf("  watermark=0\n");
	else
		printf("  watermark=%d\n", r->prod.watermark);
	printf("  bulk_default=%d\n", r->prod.bulk_default);

	/* sum and dump statistics */
#ifdef RTE_LIBRTE_RING_DEBUG
	memset(&sum, 0, sizeof(sum));
	
		sum.enq_success_bulk += r->stats.enq_success_bulk;
		sum.enq_success_objs += r->stats.enq_success_objs;
		sum.enq_quota_bulk += r->stats.enq_quota_bulk;
		sum.enq_quota_objs += r->stats.enq_quota_objs;
		sum.enq_fail_bulk += r->stats.enq_fail_bulk;
		sum.enq_fail_objs += r->stats.enq_fail_objs;
		sum.deq_success_bulk += r->stats.deq_success_bulk;
		sum.deq_success_objs += r->stats.deq_success_objs;
		sum.deq_fail_bulk += r->stats.deq_fail_bulk;
		sum.deq_fail_objs += r->stats.deq_fail_objs;

	printf("  size=%"PRIu32"\n", r->prod.size);
	printf("  enq_success_bulk=%"PRIu64"\n", sum.enq_success_bulk);
	printf("  enq_success_objs=%"PRIu64"\n", sum.enq_success_objs);
	printf("  enq_quota_bulk=%"PRIu64"\n", sum.enq_quota_bulk);
	printf("  enq_quota_objs=%"PRIu64"\n", sum.enq_quota_objs);
	printf("  enq_fail_bulk=%"PRIu64"\n", sum.enq_fail_bulk);
	printf("  enq_fail_objs=%"PRIu64"\n", sum.enq_fail_objs);
	printf("  deq_success_bulk=%"PRIu64"\n", sum.deq_success_bulk);
	printf("  deq_success_objs=%"PRIu64"\n", sum.deq_success_objs);
	printf("  deq_fail_bulk=%"PRIu64"\n", sum.deq_fail_bulk);
	printf("  deq_fail_objs=%"PRIu64"\n", sum.deq_fail_objs);
#else
	printf("  no statistics available\n");
#endif
}


