#ifndef _lib_memory_h
#define _lib_memory_h

#define UNUSED_PARAM(size) ((void)(size))

#ifdef LINUX /* Linux kernel */
#include <linux/slab.h>
#define lib_malloc(size, name) kmalloc(size, GFP_KERNEL)
#define lib_free(ptr, size, name) do { UNUSED_PARAM(size); \
                                    UNUSED_PARAM(name); kfree(ptr); } while(0)
#else
#include <stdlib.h>
#define lib_malloc(size, name) malloc(size)
#define lib_free(ptr, size, name) do { UNUSED_PARAM(size); \
                                    UNUSED_PARAM(name); free(ptr); } while(0)

#endif

#endif
