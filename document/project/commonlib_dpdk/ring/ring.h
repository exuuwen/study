#ifndef _RING_H_
#define _RING_H_
#include "../CommonLib.h"
#include "../atomic/atomic.h"
#include "../unlikely/unlikely.h"
#include "../rwlock/rwlock.h"
/**
 * @file
 * RTE Ring
 *
 * The Ring Manager is a fixed-size queue, implemented as a table of
 * pointers. Head and tail pointers are modified atomically, allowing
 * concurrent access to it. It has the following features:
 *
 * - FIFO (First In First Out)
 * - Maximum size is fixed; the pointers are stored in a table.
 * - Lockless implementation.
 * - Multi- or single-consumer dequeue.
 * - Multi- or single-producer enqueue.
 * - Bulk dequeue.
 * - Bulk enqueue.
 *
 * Note: the ring implementation is not preemptable. A lcore must not
 * be interrupted by another task that uses the same ring.
 */
 
 #ifdef RTE_LIBRTE_RING_DEBUG
/**
 * A structure that stores the ring statistics (per-lcore).
 */
struct rte_ring_debug_stats {
	uint64_t enq_success_bulk; /**< Successful enqueues number. */
	uint64_t enq_success_objs; /**< Objects successfully enqueued. */
	uint64_t enq_quota_bulk;   /**< Successful enqueues above watermark. */
	uint64_t enq_quota_objs;   /**< Objects enqueued above watermark. */
	uint64_t enq_fail_bulk;    /**< Failed enqueues number. */
	uint64_t enq_fail_objs;    /**< Objects that failed to be enqueued. */
	uint64_t deq_success_bulk; /**< Successful dequeues number. */
	uint64_t deq_success_objs; /**< Objects successfully dequeued. */
	uint64_t deq_fail_bulk;    /**< Failed dequeues number. */
	uint64_t deq_fail_objs;    /**< Objects that failed to be dequeued. */
};
#endif


#define LIB_RING_NAMESIZE 32 /**< The maximum length of a ring name. */




/**
 * An ring structure.
 *
 * The producer and the consumer have a head and a tail index. The particularity
 * of these index is that they are not between 0 and size(ring). These indexes
 * are between 0 and 2^32, and we mask their value when we access the ring[]
 * field. Thanks to this assumption, we can do subtractions between 2 index
 * values in a modulo-32bit base: that's why the overflow of the indexes is not
 * a problem.
 */
struct lib_ring {

	char name[LIB_RING_NAMESIZE];    /**< Name of the ring. */
	int flags;                       /**< Flags supplied at creation. */

	/* producer status */
	struct {
		volatile uint32_t bulk_default; /**< Default bulk count. */
		uint32_t watermark;      /**< Maximum items before EDQUOT. */
		uint32_t sp_enqueue;     /**< True, if single producer. */
		uint32_t size;           /**< Size of ring. */
		uint32_t mask;           /**< Mask (size-1) of ring. */
		volatile uint32_t head;  /**< Producer head. */
		volatile uint32_t tail;  /**< Producer tail. */
	}prod;      /**< Ring producer status. */

/* consumer status */
	struct {
		volatile uint32_t bulk_default; /**< Default bulk count. */
		uint32_t sc_dequeue;     /**< True, if single consumer. */
		uint32_t size;           /**< Size of the ring. */
		uint32_t mask;           /**< Mask (size-1) of ring. */
		volatile uint32_t head;  /**< Consumer head. */
		volatile uint32_t tail;  /**< Consumer tail. */
	}cons;      /**< Ring consumer status. */
	


#ifdef RTE_LIBRTE_RING_DEBUG
	struct rte_ring_debug_stats stats;
#endif

	void *ring[0]; /**< Memory space of ring starts here. */
};

#define RING_F_SP_ENQ 0x0001 /**< The default enqueue is "single-producer". */
#define RING_F_SC_DEQ 0x0002 /**< The default dequeue is "single-consumer". */


#ifdef RTE_LIBRTE_RING_DEBUG
#define __RING_STAT_ADD(r, name, n) do {		\
		r->stats.name##_objs += n;	\
		r->stats.name##_bulk += 1;	\
	} while(0)
#else
#define __RING_STAT_ADD(r, name, n) do {} while(0)
#endif

void lib_ring_destruct(struct lib_ring* r);

/**
 * Create a new ring named *name* in memory.
 *
 * This function uses ``memzone_reserve()`` to allocate memory. Its size is
 * set to *count*, which must be a power of two. Water marking is
 * disabled by default. The default bulk count is initialized to 1.
 * Note that the real usable ring size is *count-1* instead of
 * *count*.
 *
 * @param name
 *   The name of the ring.
 * @param count
 *   The size of the ring (must be a power of 2).
 * @param flags
 *   An OR of the following:
 *    - RING_F_SP_ENQ: If this flag is set, the default behavior when
 *      using ``rte_ring_enqueue()`` or ``rte_ring_enqueue_bulk()``
 *      is "single-producer". Otherwise, it is "multi-producers".
 *    - RING_F_SC_DEQ: If this flag is set, the default behavior when
 *      using ``rte_ring_dequeue()`` or ``rte_ring_dequeue_bulk()``
 *      is "single-consumer". Otherwise, it is "multi-consumers".
 * @return
 *   - On success, the pointer to the new allocated ring.
 *   - NULL on error (not enough memory or count is not a power of two).
 */
struct lib_ring *lib_ring_create(const char *name, unsigned count, unsigned flags);

/**
 * Set the default bulk count for enqueue/dequeue.
 *
 * The parameter *count* is the default number of bulk elements to
 * get/put when using ``rte_ring_*_{en,de}queue_bulk()``. It must be
 * greater than 0 and less than half of the ring size.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param count
 *   A new water mark value.
 * @return
 *   - 0: Success; default_bulk_count changed.
 *   - -EINVAL: Invalid count value.
 */
static inline int
lib_ring_set_bulk_count(struct lib_ring *r, unsigned count)
{
	if (unlikely(count == 0 || count >= r->prod.size))
		return -EINVAL;

	r->prod.bulk_default = r->cons.bulk_default = count;
	return 0;
}

/**
 * Get the default bulk count for enqueue/dequeue.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   The default bulk count for enqueue/dequeue.
 */
static inline unsigned
lib_ring_get_bulk_count(struct lib_ring *r)
{
	return r->prod.bulk_default;
}

/**
 * Change the high water mark.
 *
 * If *count* is 0, water marking is disabled. Otherwise, it is set to the
 * *count* value. The *count* value must be greater than 0 and less
 * than the ring size.
 *
 * This function can be called at any time (not necessarilly at
 * initialization).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param count
 *   The new water mark value.
 * @return
 *   - 0: Success; water mark changed.
 *   - -EINVAL: Invalid water mark value.
 */
int lib_ring_set_water_mark(struct lib_ring *r, unsigned count);

/**
 * Dump the status of the ring to the console.
 *
 * @param r
 *   A pointer to the ring structure.
 */
void lib_ring_dump(const struct lib_ring *r);

/**
 * Enqueue several objects on the ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table. The
 *   value must be strictly positive.
 * @return
 *   - 0: Success; objects enqueue.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue, no object is enqueued.
 */
static inline int
lib_ring_mp_enqueue_bulk(struct lib_ring *r, void * const *obj_table,
			 unsigned n)
{
	uint32_t prod_head, prod_next;
	uint32_t cons_tail, free_entries;
	int success;
	unsigned i;
	uint32_t mask = r->prod.mask;
	int ret;

	/* move prod.head atomically */
	do {
		prod_head = r->prod.head;
		cons_tail = r->cons.tail;
		/* The subtraction is done between two unsigned 32bits value
		 * (the result is always modulo 32 bits even if we have
		 * prod_head > cons_tail). So 'free_entries' is always between 0
		 * and size(ring)-1. */
		free_entries = (mask + cons_tail - prod_head);

		/* check that we have enough room in ring */
		if (unlikely(n > free_entries)) {
			__RING_STAT_ADD(r, enq_fail, n);
			return -ENOBUFS;
		}

		prod_next = prod_head + n;
		success = atomic_cmpset(&r->prod.head, prod_head,
					      prod_next);
	} while (unlikely(success == 0));

	/* write entries in ring */
	for (i = 0; likely(i < n); i++)
		r->ring[(prod_head + i) & mask] = obj_table[i];
	wmb();

	/* return -EDQUOT if we exceed the watermark */
	if (unlikely(((mask + 1) - free_entries + n) > r->prod.watermark)) {
		ret = -EDQUOT;
		__RING_STAT_ADD(r, enq_quota, n);
	}
	else {
		ret = 0;
		__RING_STAT_ADD(r, enq_success, n);
	}

	/*
	 * If there are other enqueues in progress that preceeded us,
	 * we need to wait for them to complete
	 */
	while (unlikely(r->prod.tail != prod_head))
		;

	r->prod.tail = prod_next;
	return ret;
}

/**
 * Enqueue several objects on a ring (NOT multi-producers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table. The
 *   value must be strictly positive.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int
lib_ring_sp_enqueue_bulk(struct lib_ring *r, void * const *obj_table,
			 unsigned n)
{
	uint32_t prod_head, cons_tail;
	uint32_t prod_next, free_entries;
	unsigned i;
	uint32_t mask = r->prod.mask;
	int ret;

	prod_head = r->prod.head;
	cons_tail = r->cons.tail;
	/* The subtraction is done between two unsigned 32bits value
	 * (the result is always modulo 32 bits even if we have
	 * prod_head > cons_tail). So 'free_entries' is always between 0
	 * and size(ring)-1. */
	free_entries = mask + cons_tail - prod_head;

	/* check that we have enough room in ring */
	if (unlikely(n > free_entries)) {
		__RING_STAT_ADD(r, enq_fail, n);
		return -ENOBUFS;
	}

	prod_next = prod_head + n;
	r->prod.head = prod_next;

	/* write entries in ring */
	for (i = 0; likely(i < n); i++)
	{
		r->ring[(prod_head + i) & mask] = obj_table[i];
		//printf("obj_table[%d]@0x%p\n", i, obj_table[i]);	
	}
	wmb();

	/* return -EDQUOT if we exceed the watermark */
	if (unlikely(((mask + 1) - free_entries + n) > r->prod.watermark)) {
		ret = -EDQUOT;
		__RING_STAT_ADD(r, enq_quota, n);
	}
	else {
		ret = 0;
		__RING_STAT_ADD(r, enq_success, n);
	}

	r->prod.tail = prod_next;
	return ret;
}

/**
 * Enqueue several objects on a ring.
 *
 * This function calls the multi-producer or the single-producer
 * version depending on the default behavior that was specified at
 * ring creation time (see flags).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int
lib_ring_enqueue_bulk(struct lib_ring *r, void * const *obj_table,
		      unsigned n)
{
	if (r->prod.sp_enqueue)
		return lib_ring_sp_enqueue_bulk(r, obj_table, n);
	else
		return lib_ring_mp_enqueue_bulk(r, obj_table, n);
}

/**
 * Enqueue one object on a ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj
 *   A pointer to the object to be added.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int
lib_ring_mp_enqueue(struct lib_ring *r, void *obj)
{
	return lib_ring_mp_enqueue_bulk(r, &obj, 1);
}

/**
 * Enqueue one object on a ring (NOT multi-producers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj
 *   A pointer to the object to be added.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int
lib_ring_sp_enqueue(struct lib_ring *r, void *obj)
{
	return lib_ring_sp_enqueue_bulk(r, &obj, 1);
}

/**
 * Enqueue one object on a ring.
 *
 * This function calls the multi-producer or the single-producer
 * version, depending on the default behaviour that was specified at
 * ring creation time (see flags).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj
 *   A pointer to the object to be added.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int
lib_ring_enqueue(struct lib_ring *r, void *obj)
{
	if (r->prod.sp_enqueue)
		return lib_ring_sp_enqueue(r, obj);
	else
		return lib_ring_mp_enqueue(r, obj);
}

/**
 * Dequeue several objects from a ring (multi-consumers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table,
 *   must be strictly positive
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 */
static inline int
lib_ring_mc_dequeue_bulk(struct lib_ring *r, void **obj_table, unsigned n)
{
	uint32_t cons_head, prod_tail;
	uint32_t cons_next, entries;
	int success;
	unsigned i;
	uint32_t mask = r->prod.mask;

	/* move cons.head atomically */
	do {
		cons_head = r->cons.head;
		prod_tail = r->prod.tail;
		/* The subtraction is done between two unsigned 32bits value
		 * (the result is always modulo 32 bits even if we have
		 * cons_head > prod_tail). So 'entries' is always between 0
		 * and size(ring)-1. */
		entries = (prod_tail - cons_head);

		/* check that we have enough entries in ring */
		if (unlikely(n > entries)) {
			__RING_STAT_ADD(r, deq_fail, n);
			return -ENOENT;
		}

		cons_next = cons_head + n;
		success = atomic_cmpset(&r->cons.head, cons_head,
					      cons_next);
	} while (unlikely(success == 0));

	/* copy in table */
	rmb();
	for (i = 0; likely(i < n); i++) {
		obj_table[i] = r->ring[(cons_head + i) & mask];
	}

	/*
	 * If there are other dequeues in progress that preceeded us,
	 * we need to wait for them to complete
	 */
	while (unlikely(r->cons.tail != cons_head))
		;

	__RING_STAT_ADD(r, deq_success, n);
	r->cons.tail = cons_next;
	return 0;
}

/**
 * Dequeue several objects from a ring (NOT multi-consumers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table,
 *   must be strictly positive.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 */
static inline int
lib_ring_sc_dequeue_bulk(struct lib_ring *r, void **obj_table, unsigned n)
{
	uint32_t cons_head, prod_tail;
	uint32_t cons_next, entries;
	unsigned i;
	uint32_t mask = r->prod.mask;

	cons_head = r->cons.head;
	prod_tail = r->prod.tail;
	/* The subtraction is done between two unsigned 32bits value
	 * (the result is always modulo 32 bits even if we have
	 * cons_head > prod_tail). So 'entries' is always between 0
	 * and size(ring)-1. */
	entries = prod_tail - cons_head;

	/* check that we have enough entries in ring */
	if (unlikely(n > entries)) {
		__RING_STAT_ADD(r, deq_fail, n);
		return -ENOENT;
	}

	cons_next = cons_head + n;
	r->cons.head = cons_next;

	/* copy in table */
	rmb();
	for (i = 0; likely(i < n); i++) {
		obj_table[i] = r->ring[(cons_head + i) & mask];
	}

	__RING_STAT_ADD(r, deq_success, n);
	r->cons.tail = cons_next;
	return 0;
}

/**
 * Dequeue several objects from a ring.
 *
 * This function calls the multi-consumers or the single-consumer
 * version, depending on the default behaviour that was specified at
 * ring creation time (see flags).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue, no object is
 *     dequeued.
 */
static inline int
lib_ring_dequeue_bulk(struct lib_ring *r, void **obj_table, unsigned n)
{
	if (r->cons.sc_dequeue)
		return lib_ring_sc_dequeue_bulk(r, obj_table, n);
	else
		return lib_ring_mc_dequeue_bulk(r, obj_table, n);
}

/**
 * Dequeue one object from a ring (multi-consumers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_p
 *   A pointer to a void * pointer (object) that will be filled.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 */
static inline int
lib_ring_mc_dequeue(struct lib_ring *r, void **obj_p)
{
	return lib_ring_mc_dequeue_bulk(r, obj_p, 1);
}

/**
 * Dequeue one object from a ring (NOT multi-consumers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_p
 *   A pointer to a void * pointer (object) that will be filled.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue, no object is
 *     dequeued.
 */
static inline int
lib_ring_sc_dequeue(struct lib_ring *r, void **obj_p)
{
	return lib_ring_sc_dequeue_bulk(r, obj_p, 1);
}

/**
 * Dequeue one object from a ring.
 *
 * This function calls the multi-consumers or the single-consumer
 * version depending on the default behaviour that was specified at
 * ring creation time (see flags).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_p
 *   A pointer to a void * pointer (object) that will be filled.
 * @return
 *   - 0: Success, objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue, no object is
 *     dequeued.
 */
static inline int
lib_ring_dequeue(struct lib_ring *r, void **obj_p)
{
	if (r->cons.sc_dequeue)
		return lib_ring_sc_dequeue_bulk(r, obj_p, 1);
	else
		return lib_ring_mc_dequeue_bulk(r, obj_p, 1);
}

/**
 * Test if a ring is full.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   - 1: The ring is full.
 *   - 0: The ring is not full.
 */
static inline int
lib_ring_full(const struct lib_ring *r)
{
	uint32_t prod_tail_plus_1 = (r->prod.tail + 1) & r->prod.mask;
	uint32_t cons_tail = r->cons.tail;
	return !!(prod_tail_plus_1 == cons_tail);
}

/**
 * Test if a ring is empty.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   - 1: The ring is empty.
 *   - 0: The ring is not empty.
 */
static inline int
lib_ring_empty(const struct lib_ring *r)
{
	uint32_t prod_tail = r->prod.tail;
	uint32_t cons_tail = r->cons.tail;
	return !!(cons_tail == prod_tail);
}

/**
 * Return the number of entries in a ring.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   The number of entries in the ring.
 */
static inline unsigned
lib_ring_count(const struct lib_ring *r)
{
	uint32_t prod_tail = r->prod.tail;
	uint32_t cons_tail = r->cons.tail;
	return ((prod_tail - cons_tail) & r->prod.mask);
}

/**
 * Return the number of free entries in a ring.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   The number of free entries in the ring.
 */
static inline unsigned
lib_ring_free_count(const struct lib_ring *r)
{
	uint32_t prod_tail = r->prod.tail;
	uint32_t cons_tail = r->cons.tail;
	return ((cons_tail - prod_tail - 1) & r->prod.mask);
}


#endif /* _RING_H_ */
