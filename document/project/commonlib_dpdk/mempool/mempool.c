#include "mempool.h"

LIST_HEAD(lib_mempool_list, lib_mempool);

/* global list of mempool (used for debug/dump) */
static struct lib_mempool_list mempool_list = LIST_HEAD_INITIALIZER(mempool_list);

/* find next power of 2 */
static uint32_t align32pow2(uint32_t x)
{
     x--;
     x |= x >> 1;
     x |= x >> 2;
     x |= x >> 4;
     x |= x >> 8;
     x |= x >> 16;
     return x + 1;
}


/* create the mempool */
struct lib_mempool *
lib_mempool_create(const char *name, unsigned n, unsigned elt_size,
		   unsigned private_data_size, lib_mempool_ctor_t *mp_init, void *mp_init_arg,
		   lib_mempool_obj_ctor_t *obj_init, void *obj_init_arg, unsigned flags)
{
	char mz_name[LIB_MEMZONE_NAMESIZE];
	char rg_name[LIB_RING_NAMESIZE];
	struct lib_mempool *mp;
	struct lib_ring *r;
	const struct lib_memzone *mz;
	size_t mempool_size;
	int mz_flags = 0;
	int rg_flags = 0;
	uint32_t header_size, trailer_size;
	uint32_t total_elt_size;
	unsigned i;
	void *obj;


	/* ring flags */
	if (flags & MEMPOOL_F_SP_PUT)
		rg_flags |= RING_F_SP_ENQ;
	if (flags & MEMPOOL_F_SC_GET)
		rg_flags |= RING_F_SC_DEQ;

	/* allocate the ring that will be used to store objects */
	snprintf(rg_name, sizeof(rg_name), "MP_%s", name);
	r = lib_ring_create(rg_name, align32pow2(n+1), rg_flags);
	if (r == NULL)
		return NULL;

	/*
	 * In header, we have at least the pointer to the pool, and
	 * optionaly a 64 bits cookie.
	 */
	header_size = 0;
	header_size += sizeof(struct lib_mempool *); /* ptr to pool */

	/* trailer contains the cookie in debug mode */
	trailer_size = 0;
	
	elt_size = (elt_size + 7) & (~7);

	/* this is the size of an object, including header and trailer */
	total_elt_size = header_size + elt_size + trailer_size;

	mempool_size = total_elt_size * n +
		sizeof(struct lib_mempool) + private_data_size;
	snprintf(mz_name, sizeof(mz_name), "MP_%s", name);
	mz = lib_memzone_reserve(mz_name, mempool_size, mz_flags);

	/*
	 * no more memory: in this case we loose previously reserved
	 * space for the as we cannot free it
	 */
	if (mz == NULL)
		return NULL;

	/* init the mempool structure */
	mp = mz->addr;
	memset(mp, 0, sizeof(*mp));
	snprintf(mp->name, sizeof(mp->name), "%s", name);
	mp->phys_addr = mz->phys_addr;
	mp->ring = r;
	mp->size = n;
	mp->flags = flags;
	mp->bulk_default = 1;
	mp->elt_size = elt_size;
	mp->header_size = header_size;
	mp->trailer_size = trailer_size;
	mp->private_data_size = private_data_size;

	/* call the initializer */
	if (mp_init)
		mp_init(mp, mp_init_arg);

	/* fill the headers and trailers, and add objects in ring */
	obj = (char *)mp + sizeof(struct lib_mempool) + private_data_size;
	for (i = 0; i < n; i++) {
		struct lib_mempool **mpp;
		obj = (char *)obj + header_size;

		/* set mempool ptr in header */
		mpp = __mempool_from_obj(obj);
		*mpp = mp;

		/* call the initializer */
		if (obj_init)
			obj_init(mp, obj_init_arg, obj, i);

		/* enqueue in ring */
		lib_ring_sp_enqueue(mp->ring, obj);
		obj = (char *)obj + elt_size + trailer_size;
	}

	LIST_INSERT_TAIL(&mempool_list, mp, next);
	return mp;
}

/* Return the number of entries in the mempool */
unsigned
lib_mempool_count(const struct lib_mempool *mp)
{
	unsigned count;

	count = lib_ring_count(mp->ring);

	/*
	 * due to race condition (access to len is not locked), the
	 * total can be greater than size... so fix the result
	 */
	if (count > mp->size)
		return mp->size;
	return count;
}

/* return physical address of an object */
phys_addr_t
lib_mempool_virt2phy(const struct lib_mempool *mp, const void *elt)
{
	unsigned off;

	off = (char *)elt - (char *)mp;
	return mp->phys_addr + off;
}

/* dump the status of the mempool on the console */
void
lib_mempool_dump(const struct lib_mempool *mp)
{

	unsigned common_count;
	unsigned cache_count;

	printf("mempool <%s>@%p\n", mp->name, mp);
	printf("  flags=%x\n", mp->flags);
	printf("  ring=<%s>@%p\n", mp->ring->name, mp->ring);
	printf("  size=%"PRIu32"\n", mp->size);
	printf("  bulk_default=%"PRIu32"\n", mp->bulk_default);
	printf("  header_size=%"PRIu32"\n", mp->header_size);
	printf("  elt_size=%"PRIu32"\n", mp->elt_size);
	printf("  trailer_size=%"PRIu32"\n", mp->trailer_size);
	printf("  total_obj_size=%"PRIu32"\n",
	       mp->header_size + mp->elt_size + mp->trailer_size);

	common_count = lib_ring_count(mp->ring);
	if ((cache_count + common_count) > mp->size)
		common_count = mp->size - cache_count;
	printf("  common_pool_count=%u\n", common_count);

}

/* dump the status of all mempools on the console */
void
lib_mempool_list_dump(void)
{
	const struct lib_mempool *mp;

	LIST_FOREACH(mp, &mempool_list, next) {
		lib_mempool_dump(mp);
	}
}

/* search a mempool from its name */
struct lib_mempool *
lib_mempool_lookup(const char *name)
{
	struct lib_mempool *mp;

	LIST_FOREACH(mp, &mempool_list, next) {
		if (strncmp(name, mp->name, RTE_MEMPOOL_NAMESIZE) == 0)
			break;
	}
	return mp;
}


