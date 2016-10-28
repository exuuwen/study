/*******************************************************************************

                     Copyright(c) by Ericsson Technologes

All rights reserved. No part of this document may  be  reproduced  in  any  way,
or  by  any means, without the prior written permission of Ericsson Technologies.

                  Licensed to Ericsson Technologies(tm) 2009

Description:
Hugepage is used to improve the system memory performance with high TLB cache hit. The memory 
allocated is used for memory pool or packet ring queue. The usage to big page can be with mmap
or shared memory. 
Shared memory way to use big page memory is in the shared memory class file. 
Here is the how mmap use the big page table memory.

Author: Tom, Zhang Jiangtao

History:
DD/MM/YYYY Name   Comments
11/07/2011 Tom    Initial code.
*******************************************************************************/
#ifndef _HUGE_PAGE_H_
#define _HUGE_PAGE_H_

// Default location of hugepage filesystem.
#define HUGEPAGE_MOUNT_POINT "/mnt/huge"

// Huge page information structure
typedef struct hugepage 
{
	void *orig_va;      /**< virtual addr of first mmap() */
	void *final_va;     /**< virtual addr of 2nd mmap() */
	uint32_t physaddr;  /**< physical addr */
	int file_id;        /**< the '%d' in HUGEFILE_FMT */
	int memseg_id;      /**< the memory segment to which page belongs */
} hugepage_t;

class CHugePageManager
{
public:
    CHugePageManager(int nr_pages = 1);
    ~CHugePageManager();
    
    int Open(hugepage_t ** ptable);    
    int Open(void ** addr);
    int Close();   
    void PrintPageInfo();
    
    int GetPageSize() const;
    int GetPageNumbers() const;
    
private:    
    // Common
    bool InitCheck();
    int Mount();
    int Umount();
    int Command(char * cmd);
    
    // Open mode
    int GetAllPagePhyAddr();
    int SortByPhyAddr();
    void * GetVirtualArea(int64_t * size);
    int MapAllPages();
    int UMapAllPages();
    int RemapAllPages();
    int UMapRePages();
        
private:
    // common
    int m_Page_Size;
    int m_Page_Numbers;
    
    // only used in openaswhole mode
    void * m_Addr;
    
    // only used in open mode    
    hugepage_t * m_ptable;
    
    // state machine
    bool Mounted;
    bool Opened;
};

typedef struct memseg{    
    uint32_t phys_addr;
    void * virt_addr;
    unsigned int len;
    unsigned nchannel;
    unsigned nrank;
}memseg_t;

#define MAX_MEM_SEGMENT 64
class CMemorySegmentMgr
{
public:
      CMemorySegmentMgr(hugepage_t * ptable, int pages, int pagesize);
      ~CMemorySegmentMgr();
      
      int Create();
      void Print(); 
      
      memseg_t * Reserve(unsigned int len);
       
private:
    hugepage_t * ptable;
    int pages;
    int pagesize;
    memseg_t memsegment[MAX_MEM_SEGMENT];
    int memseg_index;
    
};

#define CACHE_LINE_SIZE 64                  /**< Cache line size. */
#define CACHE_LINE_MASK (CACHE_LINE_SIZE-1) /**< Cache line mask. */

#endif
