1. RTE_MAX_LCORE
define in:
build/rte_build_config.h

config/x86/meson.build: default
dpdk_conf.set('RTE_MAX_LCORE', 128)

config/meson.build
= get_option('max_lcores')
if max_lcores == 'detect'
        # discovery makes sense only for non-cross builds
        if meson.is_cross_build()
                error('Discovery of max_lcores is not supported for cross-compilation.')
        endif
        # overwrite the default value with discovered values
        max_lcores = run_command(get_cpu_count_cmd).stdout().to_int()
        min_lcores = 2
        # DPDK must be built for at least 2 cores
        if max_lcores < min_lcores
                message('Found less than @0@ cores, building for @0@ cores'.format(min_lcores))
                max_lcores = min_lcores
        else
                message('Found @0@ cores'.format(max_lcores))
        endif
        dpdk_conf.set('RTE_MAX_LCORE', max_lcores)
elif max_lcores != 'default'
        # overwrite the default value from arch_subdir with user input
        dpdk_conf.set('RTE_MAX_LCORE', max_lcores.to_int())
endif


config:

meson --prefix=/usr -Dmax_lcores="detect"
meson --prefix=/usr -Dmax_lcores="32"

2. rte_eal_init

a. rte_eal_cpu_init: init lcore_config[RTE_MAX_LCOR].socket_id, core_id, cpuset
b. eal_parse_common_option
1). -c & -l: get core list

set lcore_config[RTE_MAX_LCORE].core_index.
init rte_config.lcore_role[RTE_MAX_LCORE] and rte_config.lcore_count

2). -a: allow device create a device_option

3). --huge-dir, --main-lcore, --file-prefix, -m, -n: store in internal_config

4). eal_adjust_config: if no --main-lcore, choose the first one, set in rte_config->main-lcore

c. eal_option_device_parse --> rte_devargs_add: create rte_devargs through device_option
rte_devargs_parse: get rte_devargs->name/bus/data  name: pci_addr, bus: pci_bus
{
 // get bus type
 rte_bus_find_by_device_name--->bus_can_parse--->pci_parse
}
bus->conf.scan_mode = RTE_BUS_SCAN_ALLOWLIST/BLOCKLIST;

d. rte_bus_scan: scan the bus through sysfs and create the device and add to the bus_list
rte_pci_scan 
{
	//ignore rte_devargs block or not allow one
	rte_pci_ignore_device
	//create rte_pci_device and add dev to rte_pci_bus.device_list
	pci_scan_one

}

e. mem/memzone init

f. pmd thread create

	pthread_setaffinity_np(pthread_self(), sizeof(rte_cpuset_t),
                       &lcore_config[config->main_lcore].cpuset); 

	//worker get thorugh rte_config->lcore_role[lcore_id] is ROLE_RTE
	RTE_LCORE_FOREACH_WORKER(i) {

                /*   
                 * create communication pipes between main thread
                 * and children
                 */
                if (pipe(lcore_config[i].pipe_main2worker) < 0) 
                        rte_panic("Cannot create pipe\n");
                if (pipe(lcore_config[i].pipe_worker2main) < 0) 
                        rte_panic("Cannot create pipe\n");

                lcore_config[i].state = WAIT;

                /* create a thread for each lcore */
                ret = pthread_create(&lcore_config[i].thread_id, NULL,
                                     eal_thread_loop, NULL);

                ret = pthread_setaffinity_np(lcore_config[i].thread_id,
                        sizeof(rte_cpuset_t), &lcore_config[i].cpuset);
        }

	rte_eal_mp_remote_launch(sync_func, NULL, SKIP_MAIN);
        rte_eal_mp_wait_lcore();


g. rte_bus_probe: probe the bus driver
//driver init first 

static struct rte_pci_driver mlx5_common_pci_driver = {
        .driver = {
                   .name = MLX5_PCI_DRIVER_NAME,
        },
        .probe = mlx5_common_pci_probe,
};

static struct mlx5_class_driver mlx5_net_driver = {
        .drv_class = MLX5_CLASS_ETH,
        .name = RTE_STR(MLX5_ETH_DRIVER_NAME),
        .id_table = mlx5_pci_id_map,
        .probe = mlx5_os_net_probe,
};

rte_mlx5_pmd_init
{
	mlx5_common_init-->mlx5_common_pci_init
	{
		rte_pci_register(&mlx5_common_pci_driver);	
	}
	mlx5_class_driver_register(&mlx5_net_driver);
}

pci_probe
{
	FOREACH_DEVICE_ON_PCIBUS(dev) {
		//driver also on the bus->driver_list: mlx5_common_pci_init
		pci_probe_all_drivers(dev)-->rte_pci_probe_one_driver
		{
			mlx5_common_pci_probe-->mlx5_os_net_probe;
		}
	}
}


3. meminit

struct rte_mem_config {
	/* memory segments and zones */
        struct rte_fbarray memzones; /**< Memzone descriptors. */

        struct rte_memseg_list memsegs[RTE_MAX_MEMSEG_LISTS];
        /**< List of dynamic arrays holding memsegs */

        struct rte_tailq_head tailq_head[RTE_MAX_TAILQ];
        /**< Tailqs for objects */

        struct malloc_heap malloc_heaps[RTE_MAX_HEAPS];
};

1). rte_eal_config_create: init mem_config share file
open & mmap "rte_eal_get_runtime_dir()/config" file for rte_config->mem_config

struct hugepage_info {
        uint64_t hugepage_sz;   /**< size of a huge page */
        char hugedir[PATH_MAX];    /**< dir where hugetlbfs is mounted */
        uint32_t num_pages[RTE_MAX_NUMA_NODES];
        /**< number of hugepages of that size on each socket */
        int lock_descriptor;    /**< file descriptor for hugepage dir */
};      

2). eal_hugepage_info_init: init huge number info
a. init internal_config->hugepage_info
hugepage_info_init
{
	scan sudir in /sys/kernel/mm/hugepages for each hugepagesz {
		scan each hugetlb mount through /proc/mounts
			if we specfic internal_conf->hugepage_dir(--huge-dir) choose this mount one
			or if the mount no pagesz the default hugepagesz one(get through /proc/meminfo)
			or choose the mount pagesz one.
	}
	
	get the num_pages(free_hugepages xx) through the hugepagesz(/sys/kernel/mm/hugepages/$pagesz) of the mount one 		
	for legacy mem store numpages in hugepage_info.num_pages[0];
	for dynamic mem store each numa pages individually through "/sys/devices/system/node/nodex/hugepages/hugepagesz/free_hugepages"
}
b. open & mmap "rte_eal_get_runtime_dir()/hugepage_info" file for internal_config->hugepage_info;

3). rte_eal_memory_init: init the memseg with hugepages

struct rte_memseg_list {
        union {
                void *base_va;
                /**< Base virtual address for this memseg list. */
                uint64_t addr_64;
                /**< Makes sure addr is always 64-bits */
        };   
        uint64_t page_sz; /**< Page size for all memsegs in this list. */
        int socket_id; /**< Socket ID for all memsegs in this list. */
        size_t len; /**< Length of memory area covered by this memseg list. */
        struct rte_fbarray memseg_arr;
};

struct rte_memseg {
        union {
                void *addr;         /**< Start virtual address. */
                uint64_t addr_64;   /**< Makes sure addr is always 64 bits */
        };
        size_t len;               /**< Length of the segment. */
        uint64_t hugepage_sz;       /**< The pagesize of underlying memory */
        int32_t socket_id;          /**< NUMA socket ID. */
} __rte_packed;


a. rte_eal_memseg_init-->memseg_primary_init

/*
EAL: Detected memory type: socket_id:0 hugepage_sz:1073741824
EAL: Creating 2 segment lists: n_segs:32 socket_id:0 hugepage_sz:1073741824
*/
init the rte_config->mem_conifg.memsegs[idx]
eal_memseg_list_init_named
{
	//memseg_arr is fbarray and store the rte_memseg
        if (rte_fbarray_init(&msl->memseg_arr, name, n_segs,
                        sizeof(struct rte_memseg))) {
                return -1;
        }    

        msl->page_sz = page_sz;
        msl->socket_id = socket_id;
        msl->base_va = NULL;
        msl->heap = heap;
}

allocate vaddr for memseg list
/*
EAL: Memseg list allocated at socket 0, page size 0x100000kB
EAL: VA reserved for memseg list at 0x140000000, size 800000000 mls 0x100000088
EAL: Memseg list allocated at socket 0, page size 0x100000kB
EAL: VA reserved for memseg list at 0x980000000, size 800000000 mls 0x100000110
*/
eal_memseg_list_alloc
{
	msl->base_va = addr;
        msl->len = mem_sz;
}

b. eal_memalloc_init: init fd_list
rte_memseg_list_walk(fd_list_create_walk, NULL)

c. rte_eal_hugepage_init
legacy mem
struct hugepage_file {
        void *orig_va;      /**< virtual addr of first mmap() */
        void *final_va;     /**< virtual addr of 2nd mmap() */
        uint64_t physaddr;  /**< physical addr */
        size_t size;        /**< the page size */
        int socket_id;      /**< NUMA socket ID */
        int file_id;        /**< the '%d' in HUGEFILE_FMT */
        char filepath[MAX_HUGEPAGE_PATH]; /**< path to backing file on filesystem */
};  
i) eal_legacy_hugepage_init
{
	//legacy mode all pages in num_pages[0]
	nr_hugepages += internal_conf->hugepage_info[i].num_pages[0]
	tmp_hp = malloc(nr_hugepages * sizeof(struct hugepage_file));
	
	/*
	create all the hugepages. 
	a. 100G will create 100 file and spend long time 
	b. "mount -t hugetlbfs none /mnt/ovsdpdk -o size=4G"  
	    will create 5 file and mmap fail for the last one
	*/
	map_all_hugepages(&tmp_hp[hp_offset], hpi, memory);
	//set physaddrs /proc/self/pagemap
	find_physaddrs(&tmp_hp[hp_offset], hpi);
	//set numa /proc/self/numa_maps
	find_numasocket(&tmp_hp[hp_offset], hpi);
	// sort the pages
	qsort(&tmp_hp[hp_offset], hpi->num_pages[0],
                      sizeof(struct hugepage_file), cmp_physaddr);

	//find the real need pages through the memory (-m or --sock-mem)
	nr_hugepages = eal_dynmem_calc_num_pages_per_socket(memory,
                        internal_conf->hugepage_info, used_hp,
                        internal_conf->num_hugepage_sizes);

	//create share memory "rte_eal_get_runtime_dir()/hugepage_data"
	create_shared_memory(eal_hugepage_data_path(),
                        nr_hugefiles * sizeof(struct hugepage_file));

	//unmap no need: there are 100G hugepage but dpdk -m 4096
	// so only need 4 file
	unmap_unneeded_hugepages();

	//remap with consecutive virtual address for consecutive physaddr
	remap_needed_hugepages(hugepage, nr_hugefiles)
	{
		//not consecutive physaddr range map one.
		remap_segment(hugepages, seg_start_page, cur_page);
		{
			for (msl_idx = 0; msl_idx < RTE_MAX_MEMSEG_LISTS; msl_idx++) {
				msl = &mcfg->memsegs[msl_idx];
                		arr = &msl->memseg_arr;
				// find the free fbarray;
				s_idx = rte_fbarray_find_next_n_free(arr, 0, seg_len + (empty ? 0 : 1))
			}

			for (cur_page = seg_start; cur_page < seg_end; cur_page++, ms_idx++) {
				// create hugapage file 
				open(hfile->filepath, O_RDWR);
				memseg_len = (size_t)page_sz;
                		addr = RTE_PTR_ADD(msl->base_va, ms_idx * memseg_len);
				//mmap the page
				addr = mmap(addr, page_sz, PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_POPULATE | MAP_FIXED, fd, 0);

				hfile->orig_va = NULL;
		                hfile->final_va = addr;

				//init the rte_memseg store in the fbarray
				s->addr = addr;
                		ms->hugepage_sz = page_sz;
                		ms->len = memseg_len;
                		ms->iova = hfile->physaddr;
                		ms->socket_id = hfile->socket_id;
			}
		}
	}
}

dynamic mem
ii) eal_dynmem_hugepage_init

eal_dynmem_calc_num_pages_per_socket: get the real num pages per socket 
{
	//memory is set -m/--socket-mem
	// the min  set mem or numa pages
	hp_used[i].num_pages[socket] = RTE_MIN(
                                        memory[socket] / hp_info[i].hugepage_sz,
                                        hp_info[i].num_pages[socket]);
}

/*
EAL: Allocating 4 pages of size 1024M on socket 0
EAL: hahaha seg_alloc list id 0 seg id 0 addr 0x140000000. sock id 0 ms 0x100035000
EAL: hahaha seg_alloc list id 0 seg id 1 addr 0x180000000. sock id 0 ms 0x100035030
EAL: hahaha seg_alloc list id 0 seg id 2 addr 0x1c0000000. sock id 0 ms 0x100035060
EAL: hahaha seg_alloc list id 0 seg id 3 addr 0x200000000. sock id 0 ms 0x100035090
*/

eal_memalloc_alloc_seg_bulk :alloc hugepage and init the memseg
{
	rte_memseg_list_walk_thread_unsafe-->alloc_seg_walk
	{
		//get free fbarray
		rte_fbarray_find_biggest_free(&cur_msl->memseg_arr, 0);
		for (i = 0; i < need; i++, cur_idx++) {
			cur = rte_fbarray_get(&cur_msl->memseg_arr, cur_idx);
                	map_addr = RTE_PTR_ADD(cur_msl->base_va,
                        	        cur_idx * page_sz);
			//allocate the hugepage memseg
			alloc_seg(cur, map_addr, wa->socket, wa->hi, msl_idx, cur_idx)
			{
				//create hugepage file and map the address
				fd = get_seg_fd(path, sizeof(path), hi, list_idx, seg_idx);
				va = mmap(addr, alloc_sz, PROT_READ | PROT_WRITE, mmap_flags, fd,
                        map_offset);
				//init the rte_memseg
				ms->addr = addr;
			        ms->hugepage_sz = alloc_sz;
        			ms->len = alloc_sz;
        			ms->socket_id = socket_id;
			}
		}
	}
}

4). rte_eal_malloc_heap_init: init the memheap from memseg

EAL: Added 4096M to heap on socket 0, msl 0x100000088 addr 0x140000000, ms 0x100035000
struct malloc_heap {
        LIST_HEAD(, malloc_elem) free_head[RTE_HEAP_NUM_FREELISTS];
        struct malloc_elem *volatile first;
        struct malloc_elem *volatile last;
                
        unsigned int alloc_count;
        unsigned int socket_id;
        size_t total_size;
        char name[RTE_HEAP_NAME_MAX_LEN];
} __rte_cache_aligned;

{
	//add all IOVA-contiguous areas to the heap
	rte_memseg_contig_walk(malloc_add_seg, NULL);
}

