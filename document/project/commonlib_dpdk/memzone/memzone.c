#include "memzone.h"

//static struct lib_memseg free_memseg[LIB_MAX_MEMSEG];
static struct lib_memseg *free_memseg;
/* pointer to last reserved memzone */
static unsigned memzone_idx;

/*
 * Return a pointer to a correctly filled memzone descriptor. If the
 * allocation cannot be done, return NULL.
 */
const struct lib_memzone *
lib_memzone_reserve(const char *name, uint64_t len, unsigned flags)
{
	struct lib_config *config;
	unsigned i = 0;
	int memseg_idx = -1;
	uint64_t memseg_len = 0;
	phys_addr_t memseg_physaddr;
	void *memseg_addr;

	/* no more room in config */
	if (memzone_idx >= LIB_MAX_MEMZONE) {
		printf("%s(): No more room in config\n", __FUNCTION__);
		return NULL;
	}

	/* get pointer to global configuration */
	config = lib_get_configuration();
	if (config == NULL) {
		printf("%s(): Cannot get config\n", __FUNCTION__);
		return NULL;
	}

	/* zone already exist */
	if (lib_memzone_lookup(name) != NULL) {
		printf("%s(): memzone <%s> already exists\n", __FUNCTION__, name);
		return NULL;
	}


	/* find the smallest segment matching requirements */
	for (i=0; i<LIB_MAX_MEMSEG; i++) {

		/* last segment */
		if (free_memseg[i].addr == NULL)
			break;

		/* empty segment, skip it */
		if (free_memseg[i].len == 0)
			continue;

		/* check len */
		if (len != 0 && len > free_memseg[i].len)
			continue;

		/* XXX check flags here when they will be used */

		/* this segment is the best until now */
		if (memseg_idx == -1) 
		{
			memseg_idx = i;
			memseg_len = free_memseg[i].len;
		}
		/* find the biggest contiguous zone */
		else if (len == 0) 
		{
			if (free_memseg[i].len > memseg_len) 
			{
				memseg_idx = i;
				memseg_len = free_memseg[i].len;
			}
		}
		/*
		 * find the smallest (we already checked that current
		 * zone length is > len
		 */
		else if (free_memseg[i].len < memseg_len) {
			memseg_idx = i;
			memseg_len = free_memseg[i].len;
		}
	}

	/* no segment found */
	if (memseg_idx == -1) 
	{
		printf("%s(): No appropriate segment found\n", __FUNCTION__);
		return NULL;
	}

	if(len == 0)
		len = memseg_len;

	/* save physical and virtual addresses */
	memseg_physaddr = free_memseg[memseg_idx].phys_addr;
	memseg_addr = free_memseg[memseg_idx].addr;

	/* update our internal state */
	free_memseg[memseg_idx].len -= len;
	free_memseg[memseg_idx].phys_addr += len;
	free_memseg[memseg_idx].addr =
		(char *)free_memseg[memseg_idx].addr + len;

	/* fill the zone in config */
	snprintf(config->memzone[memzone_idx].name,
		 sizeof(config->memzone[memzone_idx].name), "%s", name);
	config->memzone[memzone_idx].phys_addr = memseg_physaddr;
	config->memzone[memzone_idx].addr = memseg_addr;
	config->memzone[memzone_idx].len = len;
	config->memzone[memzone_idx].flags = flags; /* XXX copy flags from seg ? */

	return &config->memzone[memzone_idx++];
}


/*
 * Lookup for the memzone identified by the given name
 */
const struct lib_memzone *
lib_memzone_lookup(const char *name)
{
	const struct lib_config *config;
	unsigned i = 0;

	/* get pointer to global configuration */
	config = lib_get_configuration();
	if (config == NULL) {
		printf("%s(): Cannot get config\n", __FUNCTION__);
		return NULL;
	}

	/*
	 * the algorithm is not optimal (linear), but there are few
	 * zones and this function shoyld be called at init only
	 */
	for (i=0; i<LIB_MAX_MEMZONE; i++) 
	{
		if (!strncmp(name, config->memzone[i].name, LIB_MEMZONE_NAMESIZE))
			return &config->memzone[i];
	}
	return NULL;
}

/* Dump all reserved memory zones on console */
void
lib_memzone_dump(void)
{
	const struct lib_config *config;
	unsigned i = 0;

	/* get pointer to global configuration */
	config = lib_get_configuration();
	if (config == NULL) {
		printf("%s(): Cannot get config\n", __FUNCTION__);
		return;
	}

	/* dump all zones */
	for (i=0; i<LIB_MAX_MEMZONE; i++) 
	{
		if (config->memzone[i].addr == NULL)
			break;
		printf("name:<%s>, phys:0x%llx, virt:%p, len:0x%llx\n", config->memzone[i].name, config->memzone[i].phys_addr, config->memzone[i].addr, config->memzone[i].len);
	}
	printf("\n");
}

/*
 * Init the memzone subsystem
 */
int
lib_memzone_init(void)
{
	struct lib_config *config;
	//const struct lib_memseg *memseg;
	unsigned i = 0;

	/* get pointer to global configuration */
	config = lib_get_configuration();
	if (config == NULL) {
		printf("%s(): Cannot get config\n", __FUNCTION__);
		return -1;
	}

	//memseg = config->memseg;
	free_memseg = config->memseg;
	if (free_memseg == NULL) {
		printf("%s(): Cannot get physical layout\n",
			__FUNCTION__);
		return -1;
	}

	/* duplicate the memsegs from config */
	//memcpy(free_memseg, memseg, sizeof(free_memseg));

	/* delete all zones */
	memzone_idx = 0;
	memset(config->memzone, 0, sizeof(config->memzone));

	return 0;
}


