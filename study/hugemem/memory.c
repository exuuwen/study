#include "memory.h"
static uint64_t hugepage_sz = 0;
int nrpages = 0;

int init_check_hugetlbfs(uint64_t *pagesz)
{
	char str[1024];
    	char buf[1024];
    	int found = 0;
	
    	printf("Linux userspace are using %d bits system, standard page size %d bytes\n", sizeof(void *) * 8, getpagesize());
    	
    	FILE * f = fopen("/proc/mounts", "r");
    	if (f == NULL) 
    	{
        	printf("Cannot open /proc/mounts\n");
        	return -1;
    	}

    	snprintf(str, sizeof(str), "%s hugetlbfs", HUGEPAGE_MOUNT_POINT);
    	while(fgets(buf, sizeof(buf), f) != NULL) 
    	{
        	if (strstr(buf, str)) 
        	{
            	// find the entry, break out
            		found = 1;
            		fclose(f);
            		break;
        	}
    	}
	
    	if (!found)
    	{
        	printf("Cannot find hugetlbfs in /proc/mounts, please mount it in %s in advance\n", HUGEPAGE_MOUNT_POINT);
        	return -1;
    	}
	
	printf("found hugetlbfs in /proc/mounts\n"); 
	
	static const char pathname[] = "/proc/meminfo";
	static const char numpage_line[] = "HugePages_Total:";
	static const char pagesize_line[] = "Hugepagesize:";

	char buffer[256];
	int numpages = 0, pagesize = 0;

	FILE *fd = fopen( pathname, "r" );

	while (fgets(buffer, sizeof(buffer), fd) != NULL) {
		if (numpages == 0 &&
		    strncmp(buffer, numpage_line,
			    sizeof(numpage_line) - 1) == 0)
			numpages = atoi(buffer + strlen(numpage_line));
		else if (pagesize == 0 &&
			 strncmp(buffer, pagesize_line,
				 sizeof(pagesize_line) - 1) == 0)
			pagesize = atoi(buffer + strlen(pagesize_line));
		if (pagesize > 0 && numpages > 0)
			break;
	}

	if (pagesz != NULL)
		*pagesz = pagesize * 1024ULL;

	return numpages; 
}




/*
 * Sort the hugepg_tbl by physical address (lower addresses first). We
 * use a slow algorithm, but we won't have millions of pages, and this
 * is only done at init time.
 */
static int
sort_by_physaddr(struct hugepage *hugepg_tbl, unsigned size)
{
	unsigned i, j;
	int smallest_idx;
	uint64_t smallest_addr;
	struct hugepage tmp;

	for (i=0; i<size; i++) {
		smallest_addr = 0;
		smallest_idx = -1;

		/*
		 * browse all entries starting at 'i', and find the
		 * entry with the smallest addr
		 */
		for (j=i; j<size; j++) {

			if (smallest_addr == 0 ||
			    hugepg_tbl[j].physaddr < smallest_addr) {
				smallest_addr = hugepg_tbl[j].physaddr;
				smallest_idx = j;
			}
		}

		/* should not happen */
		if (smallest_idx == -1) {
			printf("%s(): error in physaddr sorting\n",
				__FUNCTION__);
			return -1;
		}

		/* swap the 2 entries in the table */
		memcpy(&tmp, &hugepg_tbl[smallest_idx], sizeof(struct hugepage));
		memcpy(&hugepg_tbl[smallest_idx], &hugepg_tbl[i],
		       sizeof(struct hugepage));
		memcpy(&hugepg_tbl[i], &tmp, sizeof(struct hugepage));
	}
	return 0;
}

/*
 * Uses mmap to create a shared memory area for storage of data
 *Used in this file to store the hugepage file map on disk
 */
static void *
create_shared_memory(const char *filename, const size_t mem_size)
{
	void *retval;
	int fd = open(filename, O_CREAT | O_RDWR, 0666);
	if (fd < 0)
		return NULL;
	if (ftruncate(fd, mem_size) < 0) {
		close(fd);
		return NULL;
	}
	retval = mmap(NULL, mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
		      0);
	close(fd);
	return retval;
}


/*
 * Try to mmap *size bytes in /dev/zero. If it is succesful, return the
 * pointer to the mmap'd area and keep *size unmodified. Else, retry
 * with a smaller zone: decrease *size by hugepage_sz until it reaches
 * 0. In this case, return NULL. Note: this function returns an address
 * which is a multiple of hugepage size.
 */
static void *
get_virtual_area(int64_t *size)
{
	void *addr;
	int fd;
	long aligned_addr;

	printf("Ask a virtual area of 0x%llx bytes\n", *size);

	fd = open("/dev/zero", O_RDONLY);
	do {
		addr = mmap(NULL, (*size) + hugepage_sz, PROT_READ,
			    MAP_PRIVATE, fd, 0);
		if (addr == MAP_FAILED)
			*size -= hugepage_sz;
	} while (addr == MAP_FAILED && *size > 0);

	if (addr == MAP_FAILED) {
		printf("Cannot get a virtual area\n");
		return NULL;
	}

	munmap(addr, (*size) + hugepage_sz);
	close(fd);

	/* align addr to a huge page size boundary */
	aligned_addr = (long)addr;
	aligned_addr += (hugepage_sz - 1);
	aligned_addr &= (~(hugepage_sz - 1));
	addr = (void *)(aligned_addr);

	return addr;
}


/*
 * Mmap all hugepages of hugepage table: it first open a file in
 * hugetlbfs, then mmap() hugepage_sz data in it. If orig is set, the
 * virtual address is stored in hugepg_tbl[i].orig_va, else it is stored
 * in hugepg_tbl[i].final_va. The second mapping (when orig is 0) tries to
 * map continguous physical blocks in contiguous virtual blocks.
 */
static int
map_all_hugepages(struct hugepage *hugepg_tbl, unsigned size, int orig)
{
	int fd;
	char filename[PATH_MAX];
	unsigned i;
	void *virtaddr;
	void *vma_addr = NULL;
	int64_t vma_len = 0;

	for (i = 0; i < size; i++) {

		if (orig) {
			hugepg_tbl[i].file_id = i;
		}
		else if (vma_len == 0) {
			unsigned j, num_pages;

			/* reserve a virtual area for next contiguous
			 * physical block: count the number of
			 * contiguous physical pages. */
			for (j = i+1; j < size ; j++) {
				if (hugepg_tbl[j].physaddr !=
				    hugepg_tbl[j-1].physaddr + hugepage_sz)
					break;
			}
			num_pages = j - i;
			vma_len = num_pages * hugepage_sz;

			/* get the biggest virtual memory area up to
			 * vma_len. If it fails, vma_addr is NULL, so
			 * let the kernel provide the address. */
			vma_addr = get_virtual_area(&vma_len);
			if (vma_addr == NULL)
				vma_len = hugepage_sz;
		}

		snprintf(filename, sizeof(filename), "%s/page%d", HUGEPAGE_MOUNT_POINT, hugepg_tbl[i].file_id);

		fd = open(filename, O_CREAT | O_RDWR, 0755);
		if (fd < 0) {
			printf("%s(): open failed: %s",
				__FUNCTION__, strerror(errno));
			return -1;
		}

		virtaddr = mmap(vma_addr, hugepage_sz, PROT_READ | PROT_WRITE,
				MAP_SHARED, fd, 0);
		if (virtaddr == MAP_FAILED) {
			printf("%s(): mmap failed: %s",
				__FUNCTION__, strerror(errno));
			return -1;
		}
		if (orig) {
			hugepg_tbl[i].orig_va = virtaddr;
			memset(virtaddr, 0, hugepage_sz);
		}
		else {
			hugepg_tbl[i].final_va = virtaddr;
		}

		close(fd);
		vma_addr = (char *)vma_addr + hugepage_sz;
		vma_len -= hugepage_sz;
	}
	return 0;
}

/*
 * For each hugepage in hugepg_tbl, fill the physaddr value. We find
 * it by browsing the /proc/self/pagemap special file.
 */
static int
find_physaddr(struct hugepage *hugepg_tbl, unsigned size)
{
	int fd;
	unsigned i;
	uint64_t page;
	unsigned long virt_pfn;
	int page_size;

	/* standard page size */
	page_size = getpagesize();

	fd = open("/proc/self/pagemap", O_RDONLY);
	if (fd < 0) {
		printf("%s(): cannot open /proc/self/pagemap: %s",
			__FUNCTION__, strerror(errno));
		return -1;
	}

	for (i=0; i<size; i++) {
		off_t offset;
		virt_pfn = (unsigned long)hugepg_tbl[i].orig_va /
			page_size;
		offset = sizeof(uint64_t) * virt_pfn;
		if (lseek(fd, offset, SEEK_SET) != offset){
			printf("%s(): seek error in /proc/self/pagemap: %s",
				__FUNCTION__, strerror(errno));
			close(fd);
			return -1;
		}
		if (read(fd, &page, sizeof(uint64_t)) < 0) {
			printf("%s(): cannot read /proc/self/pagemap: %s",
				__FUNCTION__, strerror(errno));
			close(fd);
			return -1;
		}

		/*
		 * the pfn (page frame number) are bits 0-54 (see
		 * pagemap.txt in linux Documentation)
		 */
		hugepg_tbl[i].physaddr = ((page & 0x7fffffffffffffULL) *
					  page_size);
	}
	close(fd);
	return 0;
}

/* Unmap all hugepages from original mapping. */
static int
unmap_all_hugepages_orig(struct hugepage *hugepg_tbl, unsigned size)
{
	unsigned i;
	for (i=0; i<size; i++) {
		munmap(hugepg_tbl[i].orig_va, hugepage_sz);
		hugepg_tbl[i].orig_va = NULL;
	}
	return 0;
}

void lib_memseg_dump(void)
{
	int i;
	struct lib_config *config;
	config = (struct lib_config *)lib_get_configuration();
	if (config == NULL) 
	{
		printf("%s(): Cannot get config\n", __FUNCTION__);
		return ;
	}

	for(i=0; i<LIB_MAX_MEMSEG; i++)    
	{
		if(config->memseg[i].addr == NULL)
			break;
		printf("memsegment %d, phy =0x%llx, virt %p, len 0x%llx\n", i, config->memseg[i].phys_addr, config->memseg[i].addr, config->memseg[i].len);    
        }
	printf("\n");
}
/*
 * Prepare physical memory mapping: fill configuration structure with
 * these infos, return 0 on success.
 *  1. map N huge pages in separate files in hugetlbfs
 *  2. find associated physical addr
 *  3. sort all huge pages by physical address
 *  4. remap these N huge pages in the correct order
 *  5. unmap the first mapping
 *  6. fill memsegs in configuration with contiguous zones
 */
struct hugepage *hugepage;
static int
lib_hugepage_init(void)
{
	int i, j, new_memseg;
	void *addr;


	/* check that hugetlbfs is mounted */
	if ((nrpages = init_check_hugetlbfs(&hugepage_sz)) < 0)
		return -1;

	printf("nrpages is:%d, pagesize is :%lld\n", nrpages, hugepage_sz);
	
	hugepage = create_shared_memory(HUGEPAGE_INFO_PATH, nrpages * sizeof(struct hugepage));
	if (hugepage == NULL)
		return -1;
	memset(hugepage, 0, nrpages * sizeof(struct hugepage));

	if (map_all_hugepages(hugepage, nrpages, 1) < 0)
		goto fail;

	if (find_physaddr(hugepage, nrpages) < 0)
		goto fail;

	for (i = 0; i < nrpages; i++)
	{		
		printf("Page %d, phy 0x%llx, init virt %p, final virt %p\n", i, hugepage[i].physaddr, hugepage[i].orig_va, hugepage[i].final_va);			
	}	
	printf("\n");
	
	if (sort_by_physaddr(hugepage, nrpages) < 0)
		goto fail;

	if (map_all_hugepages(hugepage, nrpages, 0) < 0)
		goto fail;

	if (unmap_all_hugepages_orig(hugepage, nrpages) < 0)
		goto fail;

	for (i = 0; i < nrpages; i++)
	{		
		printf("Page %d, phy 0x%llx, init virt %p, final virt %p\n", i, hugepage[i].physaddr, hugepage[i].orig_va, hugepage[i].final_va);			
	}	
	printf("\n");

	memset(&config->memseg, 0, sizeof(config->memseg));
	j = -1;
	for (i=0; i<nrpages; i++) 
	{
		new_memseg = 0;

		/* if this is a new section, create a new memseg */
		if (i == 0)
			new_memseg = 1;
		else if ((hugepage[i].physaddr - hugepage[i-1].physaddr) !=
			 hugepage_sz)
			new_memseg = 1;
		else if (((unsigned long)hugepage[i].final_va -
		     (unsigned long)hugepage[i-1].final_va) != hugepage_sz)
			new_memseg = 1;

		if (new_memseg) 
		{
			j += 1;
			if (j == LIB_MAX_MEMSEG)
				break;

			config->memseg[j].phys_addr = hugepage[i].physaddr;
			config->memseg[j].addr = hugepage[i].final_va;
			config->memseg[j].len = hugepage_sz;
		}
		/* continuation of previous memseg */
		else 
		{
			config->memseg[j].len += hugepage_sz;
		}
		hugepage[i].memseg_id = j;
	}

	lib_memseg_dump();
	

	return 0;

fail:
	return -1;
}

void lib_memory_exit()
{
    	int i;
    	for (i = 0; i < nrpages; i++)
    	{
        	if ( hugepage[i].final_va != NULL)
        	{
            		munmap( hugepage[i].final_va, hugepage_sz);
             		hugepage[i].final_va = NULL;
       		}	
    	}	
    	printf("Ummap repages sucessfully\n");
	
	munmap(hugepage, nrpages * sizeof(struct hugepage));

}

/* init memory subsystem */
int
lib_memory_init(void)
{

	if (lib_hugepage_init() < 0)
		return -1;


	return 0;
}

