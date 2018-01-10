/*-
 *
 * Copyright(c) UCloud. All rights reserved.
 * All rights reserved.
 *
 */

#include "main.h"

int32_t main(int32_t argc, char **argv)
{
	int32_t ret = -1;
	
	ret = rte_eal_init(argc, argv);
	if(0 > ret)
	{
		rte_exit(EXIT_FAILURE, "rte_eal_init failed!\n");
	}

	argc -= ret;
	argv += ret;

	ret = udpi_parse_args(argc, argv);
	if(0 > ret)
	{
		udpi_print_usage(argv[0]);
		rte_exit(EXIT_FAILURE, "invalid user args!\n");
	}

	udpi_init();

	rte_eal_mp_remote_launch(udpi_lcore_main_loop, 
		NULL, CALL_MASTER);
	
	return 0;
}

int32_t udpi_lcore_main_loop(
	__attribute__((unused)) void *arg)
{
	uint32_t core_id = 0, i = 0;

	core_id = rte_lcore_id();

	for(i = 0; i < udpi.n_cores; i++)
	{
		struct udpi_core_params *p = &udpi.cores[i];

		if(p->core_id != core_id)
		{
			continue;
		}

		switch(p->core_type)
		{
			case UDPI_CORE_TASK:
				udpi_main_loop_task(); 
				break;
			case UDPI_CORE_IPV4_RX:
				udpi_main_loop_ipv4_rx();
				break;
			default:
				rte_panic("%s: Invalid core type for core %u\n",
					__func__, i);
		}
	}

	rte_panic("%s: Algorithmic error\n", __func__);
	return -1;
}
