#ifndef _LIB_CONFIG_H_
#define _LIB_CONFIG_H_

#include "../CommonLib.h"
//#include "../memory/memory.h"

#define LIB_MAX_MEMSEG 32
#define LIB_MAX_MEMZONE 512
#define RUNTIME_CONFIG_PATH "/var/run/lib_config"
#define LIB_MEMZONE_NAMESIZE 32


/**
 * A structure describing a memzone, which is a contiguous portion of
 * physical memory identified by a name.
 */
struct lib_memzone {

 /**< Maximum length of memory zone name. */
	char name[LIB_MEMZONE_NAMESIZE];   /**< Name of the memory zone. */

	phys_addr_t phys_addr;             /**< Start physical address. */
	void *addr;                        /**< Start virtual address. */
	uint64_t len;                      /**< Length of the memzone. */
	unsigned flags;        		/**< Characteristics of this memzone. */
};


/**
 * Physical memory segment descriptor.
 */
struct lib_memseg {
	phys_addr_t phys_addr;      /**< Start physical address. */
	void *addr;                 /**< Start virtual address. */
	uint64_t len;               /**< Length of the segment. */
	unsigned nchannel;          /**< Number of channels. */
	unsigned nrank;             /**< Number of ranks. */
};

struct lib_config {
	uint32_t version; /**< Configuration [structure] version. */
	uint32_t magic;   /**< Magic number - Sanity check. */

	/* memory segments and zones */
	struct lib_memseg memseg[LIB_MAX_MEMSEG];    /**< physmem descriptors. */
	struct lib_memzone memzone[LIB_MAX_MEMZONE];

};

struct lib_config *lib_get_configuration(void);

void lib_config_create(void);

void lib_config_destroy(void);

#endif
