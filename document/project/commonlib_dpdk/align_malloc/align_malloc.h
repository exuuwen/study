#ifndef ALIGN_MALLOC_H
#define ALIGN_MALLOC_H

#include "../CommonLib.h"
#include "../unlikely/unlikely.h"
/**
 * Allocate memory from the heap.
 *
 * This function allocates size bytes from the heap, and returns a
 * pointer to the allocated memory. The memory is not cleared. The
 * type argument is used to categorize the memory.
 *
 * @param size
 *   Size (in bytes) to be allocated.
 * @param align
 *   If 0, the return is a pointer that is suitably aligned for any kind of
 *   variable (in the same manner as malloc()).
 *   Otherwise, the return is a pointer that is a multiple of *align*. In
 *   this case, it must obviously be a power of two.
 * @return
 *   - NULL on error. Not enough memory, or invalid arguments (size is 0,
 *     align is not a power of two).
 *   - Otherwise, the pointer to the allocated object.
 */

void *align_malloc(size_t size, unsigned align);

/**
 * Allocate zero'ed memory from the heap.
 *
 * Equivalent to align_malloc() except that the memory zone is
 * initialized with zeros.
 *
 * @param size
 *   Size (in bytes) to be allocated.
 * @param align
 *   If 0, the return is a pointer that is suitably aligned for any kind of
 *   variable (in the same manner as malloc()).
 *   Otherwise, the return is a pointer that is a multiple of *align*. In
 *   this case, it must obviously be a power of two.
 * @return
 *   - NULL on error. Not enough memory, or invalid arguments (size is 0,
 *     align is not a power of two).
 *   - Otherwise, the pointer to the allocated object.
 */
void *align_zmalloc(size_t size, unsigned align);

/**
 * Frees the memory space pointed to by the provided pointer.
 *
 * This pointer must have been returned by a previous call to
 * align_malloc() or rte_zmalloc(). The behaviour of align_free() is
 * undefined if the pointer does not match this requirement.
 *
 * If the pointer is NULL, the function does nothing.
 *
 * @param ptr
 *   The pointer to memory to be freed.
 */
void align_free(void *ptr);

#endif

