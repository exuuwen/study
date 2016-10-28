#ifndef _LIB_MEMZONE_H_
#define _LIB_MEMZONE_H_

#include "../CommonLib.h"
#include "../config/config.h"

const struct lib_memzone * lib_memzone_reserve(const char *name, uint64_t len, unsigned flags);

const struct lib_memzone * lib_memzone_lookup(const char *name);

void lib_memzone_dump(void);

int lib_memzone_init(void);

#endif
