/*******************************************************************************

                     Copyright(c) by Ericsson Technologes

All rights reserved. No part of this document may  be  reproduced  in  any  way,
or  by  any means, without the prior written permission of Ericsson Technologies.

                  Licensed to Ericsson Technologies(tm) 2009

Description:
Hugepage is used to improve the system memory performance with high TLB cache hit. The memory 
allocated is used for memory pool or packet ring queue. also can be used to shared memory.

Author: Tom, Zhang Jiangtao

History:
DD/MM/YYYY Name   Comments
11/07/2011 Tom    Initial code.
*******************************************************************************/
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <sys/time.h>
#include "HugePage.h"

CHugePageManager::CHugePageManager(int nr_pages)
{
    m_Page_Numbers = nr_pages;
    
    // a private page table to store pages info
    m_ptable = new hugepage[m_Page_Numbers];
    if (m_ptable == NULL)
    {
        printf("no memory in the system\n");
        assert(0);
    }
    Mounted = false;
    Opened = false;
}

CHugePageManager::~CHugePageManager()
{
    Close();
    if (m_ptable)
    {
        delete [] m_ptable;
    }
}

// This API is used to mount the huge page file system
// Returns 0 - success, 1 - fail
int CHugePageManager::Mount()
{
    char cmd[100];
	
    sprintf(cmd, "mount -t hugetlbfs nodev %s", HUGEPAGE_MOUNT_POINT);
    if (Command(cmd))
    {
        return -1;
    }
	
    // Allocate big page dynamically with the page numbers passed in parameters
    sprintf(cmd, "echo %d > /proc/sys/vm/nr_hugepages", m_Page_Numbers);
    if (Command(cmd))
    {
        return -1;
    }
	
	Mounted = true;	
    return 0;
}

// Unmount the 
int CHugePageManager::Umount()
{
	char cmd[100];
	
	if (!Mounted)
	{
	    return -1;
	}
	
	sprintf(cmd, "umount %s", HUGEPAGE_MOUNT_POINT);
	if (Command(cmd))
	{
		return -1;
	}
	
   	sprintf(cmd, "echo %d > /proc/sys/vm/nr_hugepages", 0);
    	if (Command(cmd))
    	{
        	return -1;
    	}
	Mounted = false;
	return 0;
}

int CHugePageManager::Command(char * cmd)
{
	int ret = system(cmd);
	if (ret != 0)
	{
		printf("Command %s execution failed, ret=%d\n", cmd, ret);
		return ret;
	}
	
	printf("Command '%s' execution successfully\n", cmd);	
	return 0;		
}

// Must be called after huge page file system mounted, and page numbers configured, return true;
// otherwise, return false.
bool CHugePageManager::InitCheck()
{
    char str[1024];
    char buf[1024];
    bool found = false;
	
    printf("Linux userspace are using %d bits system, standard page size %d bytes\n", sizeof(void *) * 8, getpagesize());
    	
    FILE * f = fopen("/proc/mounts", "r");
    if (f == NULL) 
    {
        printf("Cannot open /proc/mounts\n");
        return false;
    }

    snprintf(str, sizeof(str), "%s hugetlbfs", HUGEPAGE_MOUNT_POINT);
    while(fgets(buf, sizeof(buf), f) != NULL) 
    {
        if (strstr(buf, str)) 
        {
            // find the entry, break out
            found = true;
            fclose(f);
            break;
        }
    }
	
    if (!found)
    {
        printf("Cannot find hugetlbfs in /proc/mounts, please mount it in %s in advance\n", HUGEPAGE_MOUNT_POINT);
        return false;
    }
	
    printf("found hugetlbfs in /proc/mounts\n"); 
	
    // check if huge page is already allocated.
    char meminfo_path[] = "/proc/meminfo";
    char page_numbers_all_str[] = "HugePages_Total:";
    char page_numbers_free_str[] = "HugePages_Free:";
    char page_size_str[] = "Hugepagesize:";

    char line_buf[256];
    int page_numbers_all  = 0;
    int page_numbers_free = 0;
    int page_size    = 0;

    FILE *fd = fopen(meminfo_path, "r" );
    found = false;
    while (fgets(line_buf, sizeof(line_buf), fd) != NULL) 
    {
        if (page_numbers_all == 0)
        {
            if (strncmp(line_buf, page_numbers_all_str, sizeof(page_numbers_all_str) - 1) == 0)
            {
                page_numbers_all = atoi(line_buf + strlen(page_numbers_all_str));
            }
        }
        else if (page_numbers_free == 0)
        {
            if (strncmp(line_buf, page_numbers_free_str, sizeof(page_numbers_free_str) - 1) == 0)
            {
                page_numbers_free = atoi(line_buf + strlen(page_numbers_free_str));
            }
        }
        else if (page_size == 0)
        {
            if (strncmp(line_buf, page_size_str, sizeof(page_size_str) - 1) == 0)
            {
                page_size = atoi(line_buf + strlen(page_size_str));
            }
        }
		
        if (page_size > 0 && page_numbers_all > 0 && page_numbers_free > 0)
        {
            found = true;
            break;
        }
    }
	
    if (!found)
    {
        printf("Failed to find huge page numbers set in the target machine\n");    
        return false;	    
    }
    
    // Safe check: all the huge pages allocated is to be used here 
    if (page_numbers_all != page_numbers_free)
    {
        printf("page numbers all: %d is less than page numbers free: %d \n", page_numbers_all, page_numbers_free);
        return false;
    }

    if (m_Page_Numbers != page_numbers_free)
    {
        printf("Prohibited, requested page number: %d, free huge page in the system%d\n", m_Page_Numbers, page_numbers_free);
        return false;
    }
    
    // this value is init here!!
    m_Page_Size = page_size << 10;
          
    printf("Number of huge pages %d, huge page size %d Mbytes\n", page_numbers_free, m_Page_Size >> 20);	   
    return true;
}

// This mode will take the whole big pages as a whole area to use; This kind way is usually used to 
// virtual memory continous cases without requiring physical address continous.
int CHugePageManager::Open(void ** addr)
{
    int size;
    
    int uid = getuid();
    if (uid != 0)
    {
        printf("root privilige is a must to operate huge page\n");
        return -1;
    }
    
    Mount();
    bool ok = InitCheck();
    if (!ok)
    {
        printf("Failed to init huge page system\n");
        return -1;
    }

    // Open the mounted file system
    int fd= open("/mnt/huge/pages", O_CREAT | O_RDWR, 0755);
    if (fd < 0) 
    {
        perror("Open failed");
        return -1;
    }
	
    size = m_Page_Numbers * m_Page_Size;

    // truncate the file to wanted size
    if (ftruncate(fd, size) < 0)
    {
        close(fd);
        return -1;
    }
	
    m_Addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (m_Addr == MAP_FAILED)
    {
        printf("failed to mmap hugetlbfs file\n");  
        return -1; 
    }
    printf("hugepage opened as a whole sucessfully at %p\n", m_Addr); 

    close(fd);
    *addr = m_Addr;
    
    Opened = true;
    return 0;
}

int CHugePageManager::Open(hugepage_t ** ptable)
{
    bool ok;
    
    int uid = getuid();
    if (uid != 0)
    {
        printf("root privilige is a must to operate huge page\n");
        return -1;
    }
	
    // Mount the huge page filesystem to be used and config the big page numbers in the 
    // system.
    Mount();
	
    // Double check if file system mounted and big page configured.
    ok = InitCheck();
    if (!ok)
    {
        printf("Failed to init huge page system\n");
        return -1;
    }
    
    int ret = MapAllPages();
    if (ret != 0)
    {
        return -1;
    }
    
    GetAllPagePhyAddr();
    PrintPageInfo();
    SortByPhyAddr();
    printf("after sort................\n");
    PrintPageInfo();
    ret = RemapAllPages();
    if (ret != 0)
    {
        return -1;
    }    
    PrintPageInfo();
    // better to check if sucessfully opened
    *ptable = m_ptable;
    
    Opened = true;
    return 0;	
}

// Get the physical address of each huge page. 
int CHugePageManager::GetAllPagePhyAddr()
{
    int fd;
    unsigned i;
    uint64_t page;
    unsigned long virt_pfn;
    int page_size;
    char path[255];
    
    // standard page size, usually 4k bytes.
    page_size = getpagesize();

    //sprintf(path, "/proc/%d/pagemap", getpid());
    sprintf(path, "/proc/self/pagemap");
    fd = open(path, O_RDONLY);
    if (fd < 0) 
    {
        printf("%s(): cannot open %s: %s", __FUNCTION__, path, strerror(errno));
        return -1;
    }

    for (i = 0; i < m_Page_Numbers; i++) 
    {
        off_t offset;

        // get the virtual page number mapped
        virt_pfn = (unsigned long)m_ptable[i].orig_va / page_size;
        offset = sizeof(uint64_t) * virt_pfn;

        // each virtual page number has an entry in the pagemap file
        if (lseek(fd, offset, SEEK_SET) != offset)
        {
            printf("%s(): seek error in %s: %s",__FUNCTION__, path, strerror(errno));
            close(fd);
            return -1;
        }
		
        if (read(fd, &page, sizeof(uint64_t)) < 0) 
        {
            printf("%s(): cannot read %s: %s", __FUNCTION__, path, strerror(errno));
            close(fd);
            return -1;
        }

        /*
         * the pfn (page frame number) are bits 0-54 (see
         * pagemap.txt in linux Documentation)
         */
        m_ptable[i].physaddr = ((page & 0x7fffffffffffffULL) * page_size);
        if (m_ptable[i].physaddr == 0x0ULL)
        {
            printf("fatal error: huge page not mapped to physical page\n");
            return -1;
        }
    }
    close(fd);
    return 0;	
}

// This function must be called after GetAllPagePhyAddr. It is a bubble sort algorithm.
int CHugePageManager::SortByPhyAddr()
{
    unsigned int i, j;
    int min_index;
    struct hugepage tmp;
    uint64_t phy;
    for (i = 0; i < m_Page_Numbers; i++) 
    {
        phy = m_ptable[i].physaddr;
        min_index = i;
        
        for (j = i + 1; j < m_Page_Numbers; j++)
        {
            if (m_ptable[j].physaddr < phy)
            {
                min_index = j;    
                phy = m_ptable[j].physaddr;
            }
        }
        
        // swap 
        memcpy(&tmp, &m_ptable[min_index], sizeof(struct hugepage));
        memcpy(&m_ptable[min_index], &m_ptable[i], sizeof(struct hugepage));
        memcpy(&m_ptable[i], &tmp, sizeof(struct hugepage));
    }
    return 0;
}

// Map all the huge pages, get the virtual address of each mapped page.
int CHugePageManager::MapAllPages()
{
    int i;
    int fd;
    char file[255];
    void * virt_addr;
	
    // Make all the pages mapped seperatedly
    for (i = 0; i < m_Page_Numbers; i++)
    {
        // The id must be recorded, since after sort by phys addr, order will be changed
        m_ptable[i].file_id = i;
        snprintf(file, sizeof(file), "%s/page%d", HUGEPAGE_MOUNT_POINT, i);
        fd = open(file, O_CREAT | O_RDWR, 0755);
        if (fd < 0)
        {
            printf("open fd failed %d\n", i);
            return -1;
        }
	    
        virt_addr = mmap(NULL, m_Page_Size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (virt_addr == MAP_FAILED) 
        {
            printf("%s(): mmap failed: %s\n", __FUNCTION__, strerror(errno));
            return -1;
        }
		
        printf("page %s is mapped at %p\n", file, virt_addr);	
		
        m_ptable[i].orig_va = virt_addr;

        // WARNING:
        // It is a must to do memset here, otherwise, all the pages virtual is not mapped to 
        // physical memory page. all the physical address get will be 0;
        memset(virt_addr, 0, m_Page_Size);
        close(fd);	
    }
    return 0;
}

int CHugePageManager::UMapAllPages()
{
    int i;
    for (i = 0; i < m_Page_Numbers; i++)
    {
        if (m_ptable[i].orig_va != NULL)
        {
            munmap(m_ptable[i].orig_va, m_Page_Size);
            m_ptable[i].orig_va = NULL;
        }	
    }	
    printf("Ummap all the pages sucessfully\n");
    return 0;
}

int CHugePageManager::UMapRePages()
{
    int i;
    for (i = 0; i < m_Page_Numbers; i++)
    {
        if (m_ptable[i].final_va != NULL)
        {
            munmap(m_ptable[i].final_va, m_Page_Size);
            m_ptable[i].final_va = NULL;
        }	
    }	
    printf("Ummap repages sucessfully\n");
    return 0;
}

// The reason why remap:
// Best effort to make all virtual address continous
int CHugePageManager::RemapAllPages()
{
    int i;
    int fd;
    char file[255];
    void * virt_addr;
    int64_t vma_len = 0;
    void * vma_addr = NULL;
	
    for (i = 0; i < m_Page_Numbers; i++)
    {
        if (vma_len == 0)
        {
            unsigned int j, pages;
            for (j = i + 1; j < m_Page_Numbers; j++)
            {
                if (m_ptable[j].physaddr != m_ptable[j-1].physaddr + m_Page_Size)
                {
                    break;
                }
            }
            pages = j - i;
            vma_len = pages * m_Page_Size;    
            vma_addr = GetVirtualArea(&vma_len);   
            if (vma_addr == NULL)
            {
                vma_len = m_Page_Size;
            }          
        }
        
        snprintf(file, sizeof(file), "%s/page%d", HUGEPAGE_MOUNT_POINT, m_ptable[i].file_id);
        fd = open(file, O_CREAT | O_RDWR, 0755);
        if (fd < 0)
        {
            printf("open fd failed %d\n", i);
            return -1;
        }
	    
        virt_addr = mmap(vma_addr, m_Page_Size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (virt_addr == MAP_FAILED) 
        {
            printf("%s(): mmap failed: %s\n", __FUNCTION__, strerror(errno));
            return -1;
        }
		
        printf("page %s is remapped at %p\n", file, virt_addr);	
		
        m_ptable[i].final_va = virt_addr;
        close(fd);	
        vma_addr = (char *) vma_addr + m_Page_Size;
        vma_len -= m_Page_Size;
    }
    return 0;
}

void * CHugePageManager::GetVirtualArea(int64_t * size)
{
	void *addr;
	int fd;
	long aligned_addr;
	
	fd = open("/dev/zero", O_RDONLY);
	do 
	{
        addr = mmap(NULL, (*size) + m_Page_Size, PROT_READ, MAP_PRIVATE, fd, 0);
		if (addr == MAP_FAILED)
		{
			*size -= m_Page_Size;
		}
	} while (addr == MAP_FAILED && *size > 0);

	if (addr == MAP_FAILED) 
	{
		printf("Cannot get a virtual area\n");
		return NULL;
	}

	munmap(addr, (*size) + m_Page_Size);
	close(fd);

	/* align addr to a huge page size boundary */
	aligned_addr = (long)addr;
	aligned_addr += (m_Page_Size - 1);
	aligned_addr &= (~(m_Page_Size - 1));
	addr = (void *)(aligned_addr);

	return addr;    
}

void CHugePageManager::PrintPageInfo()
{
	int i;
	uint32_t phy;
	void * vir;
	
	for (i = 0; i < m_Page_Numbers; i++)
	{
		phy = m_ptable[i].physaddr; 
		vir = m_ptable[i].orig_va;
		printf("Page %d, phy 0x%x, init virt %p, final virt %p\n", i,  phy, vir, m_ptable[i].final_va);			
	}		
}

int CHugePageManager::Close()
{	
    // if map as a whole mode 
    if (m_Addr != NULL)
    {
        munmap(m_Addr, m_Page_Size * m_Page_Numbers);
        m_Addr = NULL;
    }
	
    UMapAllPages();
    UMapRePages();
    Umount();

  
    
    Opened = false;
    printf("hugepage closed sucessfully\n");
    return 0;
}

int CHugePageManager::GetPageSize() const
{
    return m_Page_Size;
}

int CHugePageManager::GetPageNumbers() const
{
    return m_Page_Numbers;
}

////////////////////////////////////////////////////////////////////////////////
CMemorySegmentMgr::CMemorySegmentMgr(hugepage_t * table, int pages, int pagesize)
{
    this->ptable = table;
    this->pages  = pages;
    this->pagesize = pagesize;
    memseg_index = -1;
}

CMemorySegmentMgr::~CMemorySegmentMgr()
{
    
}

int CMemorySegmentMgr::Create()
{
    int i;
    for (i = 0; i < pages; i++)
    {
        bool new_mem = false;
        
        if (i == 0)
        {
            new_mem = true;
        }
        else if ((ptable[i].physaddr - ptable[i - 1].physaddr) != pagesize)  
        {
            new_mem = true;
        }
        else if (((unsigned long)ptable[i].final_va - (unsigned long)ptable[i - 1].final_va) != pagesize) 
        {
            new_mem = true;
        }
        
        if (new_mem)
        {
            memseg_index++; 
            if (memseg_index >= MAX_MEM_SEGMENT)
            {
                printf("MAX_MEM_SEGMENT is limited\n");
                break;
            }               
            memsegment[memseg_index].phys_addr = ptable[i].physaddr;
            memsegment[memseg_index].virt_addr = ptable[i].final_va;
            memsegment[memseg_index].len = pagesize;                    
        }
        else
        {
            // continous physical memory
            memsegment[memseg_index].len += pagesize;   
        }
        
        // let the page table know who is using it.
        ptable[i].memseg_id = memseg_index;       
    }    
}

void CMemorySegmentMgr::Print()
{
    for (int i = 0; i <= memseg_index; i++)    
    {
        printf("memsegment %d, phy 0x%x, virt %p, len %d\n", i, memsegment[i].phys_addr, memsegment[i].virt_addr, memsegment[i].len);    
    }
}

// Only reserve, there is no way to put these memory back.
memseg_t * CMemorySegmentMgr::Reserve(unsigned int len)
{
    int i;
    int index = -1;
    unsigned int mem_len = 0;
    // cache align the length
    len += CACHE_LINE_MASK;
    len &= ~((unsigned int)CACHE_LINE_MASK);   
    
    // find the smallest mem segment matching the request
    for (i = 0; i <= memseg_index; i++)    
    {
        if (memsegment[i].virt_addr == NULL)
        {
            break;
        }
        
        if (memsegment[i].len == 0 || memsegment[i].phys_addr == 0)
        {
            break;
        }                    
        
        if (len > memsegment[i].len)
        {
            continue;
        }
        
        // initial choice
        if (index == -1)
        {
            index = i;
            mem_len = memsegment[i].len;
        } 
        else if (memsegment[i].len < mem_len)
        {
            index = i;
            mem_len = memsegment[i].len;
        }                  
    }
    
    if (index == -1)
    {
        return NULL;
    }
    
    // create the return phy memory descriptor
    memseg_t *part = new memseg_t();
    part->phys_addr = memsegment[index].phys_addr;
    part->virt_addr = memsegment[index].virt_addr;
    part->len       = len; // the requested lenght
    // Update the internal memory segement state
    memsegment[index].phys_addr += len;
    memsegment[index].virt_addr  = (char *) memsegment[index].virt_addr + len;
    memsegment[index].len -= len;
    
    return part;       
}    

int main()
{
	CHugePageManager hugemgr(9);
	hugepage_t * table;
	void * v;
	//int ret = hugemgr.Open(&v);
	int ret = hugemgr.Open(&table);
	if (ret != 0)
	{
	    return -1;
	}
	
	int pages = hugemgr.GetPageNumbers();
	int pagesize = hugemgr.GetPageSize();
	//
	CMemorySegmentMgr memMgr(table, pages, pagesize);
	memMgr.Create();
	memMgr.Print();
	memseg_t * memseg = memMgr.Reserve(4 * 1024 * 1024);
	if (memseg == NULL)
	{
	    printf("allocate memory from mempool failed\n");
	}
	else
	{
	    printf("get memory from the pool %d\n", memseg->len);
	}
	memMgr.Print();
	//
	// doing the memory performance testing.
	char * p = new char[4 * 1024 * 1024];
	if (p == NULL)
	{
	    printf("memory is out of 4M\n");
	    return -1;
	}
	
	int l = 2 * 1024 * 1024;
	struct timeval now, end;
	int len = 0;
	char * packet= new char[1024];
	memset(packet, 0x5, 1024);
	
	// /////heap....
	gettimeofday(&now, NULL);
	while(len < l)
	{
	    memcpy(p, (void *)packet, 1024);
	    len += 1024;
	    p += 1024;    
	}
	gettimeofday(&end, NULL);
	printf("heap memory access diff: %d sec, %d usec\n", end.tv_sec - now.tv_sec, end.tv_usec - now.tv_usec);
	
	//////////big page
	len = 0;
	char * pp = (char *)memseg->virt_addr;
	//char * pp = (char *)v;
	gettimeofday(&now, NULL);
	while(len < l)
	{
	    memcpy(pp, (void*)packet, 1024);
	    len += 1024;
	    pp += 1024;    
	}
	gettimeofday(&end, NULL);
	printf("huge page memory access diff: %d sec, %d usec\n", end.tv_sec - now.tv_sec, end.tv_usec - now.tv_usec);
	return 0;
}
