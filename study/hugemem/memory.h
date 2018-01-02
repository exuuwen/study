#ifndef _LIB_MEMORY_H_
#define _LIB_MEMORY_H_

#include "../CommonLib.h"
#include "../config/config.h"


#define HUGEPAGE_MOUNT_POINT "/mnt/huge"
#define HUGEPAGE_INFO_PATH "/var/run/lib_hugepage_map"

/**
 * Structure used to store informations about hugepages that we mapped
 * through the files in hugetlbfs.
 */
struct hugepage {
	void *orig_va;      /**< virtual addr of first mmap() */
	void *final_va;     /**< virtual addr of 2nd mmap() */
	uint64_t physaddr;  /**< physical addr */
	int file_id;        /**< the '%d' in HUGEFILE_FMT */
	int memseg_id;      /**< the memory segment to which page belongs */
};




int lib_memory_init(void);

void lib_memseg_dump(void);

void lib_memory_exit(void);
#endif
