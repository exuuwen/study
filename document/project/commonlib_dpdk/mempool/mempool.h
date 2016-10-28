#ifndef _LIB_MEMPOOL_H_
#define _LIB_MEMPOOL_H_

#include "../CommonLib.h"
#include "../ring/ring.h"
#include "../queue/queue.h"
#include "../memzone/memzone.h"

#define MEMPOOL_F_SP_PUT         0x0004 /**< Default put is "single-producer".*/
#define MEMPOOL_F_SC_GET         0x0008 /**< Default get is "single-consumer".*/

#define LIB_MEMPOOL_NAMESIZE 32 


struct lib_mempool {
	LIST_ENTRY(lib_mempool) next;   /**< Next in list. */

	char name[LIB_MEMPOOL_NAMESIZE]; /**< Name of mempool. */
	struct lib_ring *ring;           /**< Ring to store objects. */
	phys_addr_t phys_addr;           /**< Phys. addr. of mempool struct. */
	int flags;                       /**< Flags of the mempool. */
	uint32_t size;                   /**< Size of the mempool. */
	uint32_t bulk_default;           /**< Default bulk count. */
	
	uint32_t elt_size;               /**< Size of an element. */
	uint32_t header_size;            /**< Size of header (before elt). */
	uint32_t trailer_size;           /**< Size of trailer (after elt). */

	unsigned private_data_size;      /**< Size of private data. */

}

typedef void (lib_mempool_ctor_t)(struct lib_mempool *, void *);

typedef void (lib_mempool_obj_ctor_t)(struct lib_mempool *, void *,
				      void *, unsigned);


/**
 * Get a pointer to a mempool pointer in the object header.
 * @param obj
 *   Pointer to object.
 * @return
 *   The pointer to the mempool from which the object was allocated.
 */
static inline struct lib_mempool **__mempool_from_obj(const void *obj)
{
	struct lib_mempool **mpp;
	unsigned off;

	off = sizeof(struct lib_mempool *);
	mpp = (struct lib_mempool **)((const char *)obj - off);
	return mpp;
}

/**
 * Return a pointer to the mempool owning this object.
 *
 * @param obj
 *   An object that is owned by a pool. If this is not the case,
 *   the behavior is undefined.
 * @return
 *   A pointer to the mempool structure.
 */
static inline struct lib_mempool *lib_mempool_from_obj(const void *obj)
{
	struct lib_mempool * const *mpp;
	mpp = __mempool_from_obj(obj);
	return *mpp;
}

struct lib_mempool *
lib_mempool_create(const char *name, unsigned n, unsigned elt_size, unsigned private_data_size,
		   lib_mempool_ctor_t *mp_init, void *mp_init_arg, lib_mempool_obj_ctor_t *obj_init, void *obj_init_arg,
		   unsigned flags);

/**
 * Set the default bulk count for put/get.
 *
 * The *count* parameter is the default number of bulk elements to
 * get/put when using ``rte_mempool_*_{en,de}queue_bulk()``. It must
 * be greater than 0 and less than half of the mempool size.
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @param count
 *   A new water mark value.
 * @return
 *   - 0: Success; default_bulk_count changed.
 *   - -EINVAL: Invalid count value.
 */
static inline int
lib_mempool_set_bulk_count(struct lib_mempool *mp, unsigned count)
{
	if (unlikely(count == 0 || count >= mp->size))
		return -EINVAL;

	mp->bulk_default = count;
	return 0;
}

/**
 * Get the default bulk count for put/get.
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @return
 *   The default bulk count for enqueue/dequeue.
 */
static inline unsigned
lib_mempool_get_bulk_count(struct lib_mempool *mp)
{
	return mp->bulk_default;
}

/**
 * Dump the status of the mempool to the console.
 *
 * @param mp
 *   A pointer to the mempool structure.
 */
void lib_mempool_dump(const struct lib_mempool *mp);

/**
 * Put several objects back in the mempool; used internally.
 * @param mp
 *   A pointer to the mempool structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to store back in the mempool, must be strictly
 *   positive.
 * @param is_mp
 *   Mono-producer (0) or multi-producers (1).
 */
static inline void
__mempool_put_bulk(struct lib_mempool *mp, void * const *obj_table,
		    unsigned n, int is_mp)
{
	if (is_mp) {
		if (lib_ring_mp_enqueue_bulk(mp->ring, obj_table, n) < 0)
			perror("cannot put objects in mempool\n");
	}
	else {
		if (lib_ring_sp_enqueue_bulk(mp->ring, obj_table, n) < 0)
			perror("cannot put objects in mempool\n");
	}

}


/**
 * Put several objects back in the mempool (multi-producers safe).
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the mempool from the obj_table.
 */
static inline void
lib_mempool_mp_put_bulk(struct lib_mempool *mp, void * const *obj_table,
			unsigned n)
{
	__mempool_put_bulk(mp, obj_table, n, 1);
}

/**
 * Put several objects back in the mempool (NOT multi-producers safe).
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the mempool from obj_table.
 */
static inline void
lib_mempool_sp_put_bulk(struct lib_mempool *mp, void * const *obj_table,
			unsigned n)
{
	__mempool_put_bulk(mp, obj_table, n, 0);
}

/**
 * Put several objects back in the mempool.
 *
 * This function calls the multi-producer or the single-producer
 * version depending on the default behavior that was specified at
 * mempool creation time (see flags).
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the mempool from obj_table.
 */
static inline void
lib_mempool_put_bulk(struct lib_mempool *mp, void * const *obj_table,
		     unsigned n)
{
	__mempool_put_bulk(mp, obj_table, n, !(mp->flags & MEMPOOL_F_SP_PUT));
}

/**
 * Put one object in the mempool (multi-producers safe).
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @param obj
 *   A pointer to the object to be added.
 */
static inline void
lib_mempool_mp_put(struct lib_mempool *mp, void *obj)
{
	lib_mempool_mp_put_bulk(mp, &obj, 1);
}

/**
 * Put one object back in the mempool (NOT multi-producers safe).
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @param obj
 *   A pointer to the object to be added.
 */
static inline void
lib_mempool_sp_put(struct lib_mempool *mp, void *obj)
{
	lib_mempool_sp_put_bulk(mp, &obj, 1);
}

/**
 * Put one object back in the mempool.
 *
 * This function calls the multi-producer or the single-producer
 * version depending on the default behavior that was specified at
 * mempool creation time (see flags).
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @param obj
 *   A pointer to the object to be added.
 */
static inline void
lib_mempool_put(struct lib_mempool *mp, void *obj)
{
	lib_mempool_put_bulk(mp, &obj, 1);
}

/**
 * Get several objects from the mempool; used internally.
 * @param mp
 *   A pointer to the mempool structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to get, must be strictly positive.
 * @param is_mc
 *   Mono-consumer (0) or multi-consumers (1).
 * @return
 *   - >=0: Success; number of objects supplied.
 *   - <0: Error; code of ring dequeue function.
 */
static inline int
__mempool_get_bulk(struct lib_mempool *mp, void **obj_table,
		   unsigned n, int is_mc)
{
	int ret;


	/* get remaining objects from ring */
	if (is_mc)
		ret = lib_ring_mc_dequeue_bulk(mp->ring, obj_table, n);
	else
		ret = lib_ring_sc_dequeue_bulk(mp->ring, obj_table, n);


	return ret;
}

/**
 * Get several objects from the mempool (multi-consumers safe).
 *
 * If cache is enabled, objects will be retrieved first from cache,
 * subsequently from the common pool. Note that it can return -ENOENT when
 * the local cache and common pool are empty, even if cache from other
 * lcores are full.
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to get from mempool to obj_table.
 * @return
 *   - 0: Success; objects put.
 *   - -ENOENT: Not enough entries in the mempool to put; no object is retrieved.
 */
static inline int
lib_mempool_mc_get_bulk(struct lib_mempool *mp, void **obj_table, unsigned n)
{
	int ret;
	ret = __mempool_get_bulk(mp, obj_table, n, 1);
	return ret;
}

/**
 * Get several objects from the mempool (NOT multi-consumers safe).
 *
 * If cache is enabled, objects will be retrieved first from cache,
 * subsequently from the common pool. Note that it can return -ENOENT when
 * the local cache and common pool are empty, even if cache from other
 * lcores are full.
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to get from the mempool to obj_table.
 * @return
 *   - 0: Success; objects put.
 *   - -ENOENT: Not enough entries in the mempool to put; no object is
 *     retrieved.
 */
static inline int
lib_mempool_sc_get_bulk(struct lib_mempool *mp, void **obj_table, unsigned n)
{
	int ret;
	ret = __mempool_get_bulk(mp, obj_table, n, 0);
	return ret;
}

/**
 * Get several objects from the mempool.
 *
 * This function calls the multi-consumers or the single-consumer
 * version, depending on the default behaviour that was specified at
 * mempool creation time (see flags).
 *
 * If cache is enabled, objects will be retrieved first from cache,
 * subsequently from the common pool. Note that it can return -ENOENT when
 * the local cache and common pool are empty, even if cache from other
 * lcores are full.
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to get from the mempool to obj_table.
 * @return
 *   - 0: Success; objects put
 *   - -ENOENT: Not enough entries in the mempool to put; no object is retrieved.
 */
static inline int
lib_mempool_get_bulk(struct lib_mempool *mp, void **obj_table, unsigned n)
{
	int ret;
	ret = __mempool_get_bulk(mp, obj_table, n,
				 !(mp->flags & MEMPOOL_F_SC_GET));
	return ret;
}

/**
 * Get one object from the mempool (multi-consumers safe).
 *
 * If cache is enabled, objects will be retrieved first from cache,
 * subsequently from the common pool. Note that it can return -ENOENT when
 * the local cache and common pool are empty, even if cache from other
 * lcores are full.
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @param obj_p
 *   A pointer to a void * pointer (object) that will be filled.
 * @return
 *   - 0: Success; objects put.
 *   - -ENOENT: Not enough entries in the mempool to put; no object is retrieved.
 */
static inline int
lib_mempool_mc_get(struct rte_mempool *mp, void **obj_p)
{
	return lib_mempool_mc_get_bulk(mp, obj_p, 1);
}

/**
 * Get one object from the mempool (NOT multi-consumers safe).
 *
 * If cache is enabled, objects will be retrieved first from cache,
 * subsequently from the common pool. Note that it can return -ENOENT when
 * the local cache and common pool are empty, even if cache from other
 * lcores are full.
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @param obj_p
 *   A pointer to a void * pointer (object) that will be filled.
 * @return
 *   - 0: Success; objects put.
 *   - -ENOENT: Not enough entries in the mempool to put; no object is retrieved.
 */
static inline int
lib_mempool_sc_get(struct lib_mempool *mp, void **obj_p)
{
	return lib_mempool_sc_get_bulk(mp, obj_p, 1);
}

/**
 * Get one object from the mempool.
 *
 * This function calls the multi-consumers or the single-consumer
 * version, depending on the default behavior that was specified at
 * mempool creation (see flags).
 *
 * If cache is enabled, objects will be retrieved first from cache,
 * subsequently from the common pool. Note that it can return -ENOENT when
 * the local cache and common pool are empty, even if cache from other
 * lcores are full.
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @param obj_p
 *   A pointer to a void * pointer (object) that will be filled.
 * @return
 *   - 0: Success; objects put.
 *   - -ENOENT: Not enough entries in the mempool to put; no object is
 *     retrieved.
 */
static inline int
lib_mempool_get(struct lib_mempool *mp, void **obj_p)
{
	return lib_mempool_get_bulk(mp, obj_p, 1);
}

/**
 * Return the number of entries in the mempool.
 *
 * When cache is enabled, this function has to browse the length of
 * all lcores, so it should not be used in a data path, but only for
 * debug purposes.
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @return
 *   The number of entries in the mempool.
 */
unsigned lib_mempool_count(const struct lib_mempool *mp);

/**
 * Return the number of free entries in the mempool.
 *
 * When cache is enabled, this function has to browse the length of
 * all lcores, so it should not be used in a data path, but only for
 * debug purposes.
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @return
 *   The number of free entries in the mempool.
 */
static inline unsigned
lib_mempool_free_count(const struct lib_mempool *mp)
{
	return mp->size - rte_mempool_count(mp);
}

/**
 * Test if the mempool is full.
 *
 * When cache is enabled, this function has to browse the length of all
 * lcores, so it should not be used in a data path, but only for debug
 * purposes.
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @return
 *   - 1: The mempool is full.
 *   - 0: The mempool is not full.
 */
static inline int
lib_mempool_full(const struct lib_mempool *mp)
{
	return !!(rte_mempool_count(mp) == mp->size);
}

/**
 * Test if the mempool is empty.
 *
 * When cache is enabled, this function has to browse the length of all
 * lcores, so it should not be used in a data path, but only for debug
 * purposes.
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @return
 *   - 1: The mempool is empty.
 *   - 0: The mempool is not empty.
 */
static inline int
rte_mempool_empty(const struct lib_mempool *mp)
{
	return !!(lib_mempool_count(mp) == 0);
}

/**
 * Return the physical address of elt, which is an element of the pool mp.
 *
 * When cache is enabled, this function has to browse the length of all
 * lcores, so it should not be used in a data path, but only for debug
 * purposes.
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @param elt
 *   A pointer (virtual address) to the element of the pool.
 * @return
 *   The physical address of the elt element.
 */
phys_addr_t lib_mempool_virt2phy(const struct lib_mempool *mp,
				 const void *elt);

/**
 * Return a pointer to the private data in an mempool structure.
 *
 * @param mp
 *   A pointer to the mempool structure.
 * @return
 *   A pointer to the private data.
 */
static inline void *lib_mempool_get_priv(struct lib_mempool *mp)
{
	return (char *)mp + sizeof(struct lib_mempool);
}

/**
 * Dump the status of all mempools on the console
 */
void lib_mempool_list_dump(void);

/**
 * Search a mempool from its name
 *
 * @param name
 *   The name of the mempool.
 * @return
 *   The pointer to the mempool matching the name, or NULL if not found.
 */
struct lib_mempool *lib_mempool_lookup(const char *name);

#endif /* _RTE_MEMPOOL_H_ */

