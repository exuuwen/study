/*-
 *
 * Copyright(c) UCloud. All rights reserved.
 * All rights reserved.
 *
 */
 
#include <getopt.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <rte_common.h>
#include <rte_debug.h>
#include <rte_lcore.h>
#include <rte_cfgfile.h>
#include <rte_malloc.h>

#include "main.h"

static const char usage[] = 
	"Usage: %s UDPI_OPTIONS -- -p [PORT_MASK] [-f CONFIG_FILE]\n";


void udpi_print_usage(char *prgname)
{
	printf(usage, prgname);

	return;
}

static const char * udpi_core_type_id_to_string(enum udpi_core_type id)
{
	switch(id)
	{
		case UDPI_CORE_NONE:
			return "NONE";
		case UDPI_CORE_TASK:
			return "TASK";
		case UDPI_CORE_IPV4_RX:
			return "IPV4_RX";
		default:
			return NULL;
	}
}


int32_t udpi_core_type_string_to_id(
	const char *string, 
	enum udpi_core_type *id)
{
	if(0 == (strcmp(string, "NONE")))
	{
		*id = UDPI_CORE_NONE;
		return 0;
	}
	
	if(0 == (strcmp(string, "TASK")))
	{
		*id = UDPI_CORE_TASK;
		return 0;
	}
	
	if(0 == (strcmp(string, "IPV4_RX")))
	{
		*id = UDPI_CORE_IPV4_RX;
		return 0;
	}

	return -1;
}


static const char * udpi_port_type_id_to_string(enum udpi_port_type id)
{
	switch(id)
	{
		case UDPI_PORT_NONE:
			return "NONE";
		case UDPI_PORT_INPUT:
			return "INPUT";
		case UDPI_PORT_OUTPUT:
			return "OUTPUT";
		default:
			return NULL;
	}
}


int32_t udpi_port_type_string_to_id(
	const char *string, 
	enum udpi_port_type *id)
{
	if (0 == (strcmp(string, "NONE")))
	{
		*id = UDPI_PORT_NONE;
		return 0;
	}
	
	if (0 == (strcmp(string, "INPUT")))
	{
		*id = UDPI_PORT_INPUT;
		return 0;
	}
	
	if (0 == (strcmp(string, "OUTPUT")))
	{
		*id = UDPI_PORT_OUTPUT;
		return 0;
	}

	return -1;
}

static int udpi_log_type_string_to_level(
	const char *string, 
	uint32_t *level)
{
	if (!strcmp(string, "EMERG"))
	{
		*level = 0;
	}
	else if (!strcmp(string, "ALERT"))
	{
		*level = 1;
	}
	else if (!strcmp(string, "CRIT"))
	{
		*level = 2;
	}
	else if (!strcmp(string, "ERR"))
	{
		*level = 3;
	}
	else if (!strcmp(string, "WARNING"))
	{
		*level = 4;
	}
	else if (!strcmp(string, "NOTICE"))
	{
		*level = 5;
	}
	else if (!strcmp(string, "INFO"))
	{
		*level = 6;
	}
	else if (!strcmp(string, "DEBUG"))
	{
		*level = 7;
	}
	else
		return -1;
	
	return 0;
}

static int32_t udpi_install_cfgfile(const char *file_name)
{
	struct rte_cfgfile *file = NULL;
	uint32_t n_cores = 0, i = 0, n_ports = 0;
	int32_t ret = -1;

	memset(udpi.cores, 0, sizeof(udpi.cores));

	if('\0' == file_name[0])
	{
		return -1;
	}

	file = rte_cfgfile_load(file_name, 0);
	if(NULL == file)
	{
		rte_panic("Config file %s not found\n", file_name);
		return -1;
	}

	n_cores = (uint32_t)rte_cfgfile_num_sections(file, 
		"core", strnlen("core", 5));
	if(n_cores < udpi.n_cores)
	{
		rte_panic("Config file parse error: not enough thread specified "
			"(%u thread missing)\n", (udpi.n_cores - n_cores));
		goto end;
	}
	if(n_cores > udpi.n_cores)
	{
		rte_panic("Config file parse error: too many thread specified "
			"(%u thread too many)\n", (n_cores - udpi.n_cores));
		goto end;
	}

	
	n_ports = (uint32_t)rte_cfgfile_num_sections(file, 
		"port", strnlen("port", 5));
	if(n_ports != udpi.n_ports)
	{
		rte_panic("Config file parse error: port specific error "
			"(%u %u)\n", udpi.n_ports, n_ports);
		goto end;
	}

	for(i = 0; i < n_cores; i++)
	{
		struct udpi_core_params *p = &udpi.cores[i];
		char section_name[16];
		const char *entry = NULL;

		p->core_total_cycles = 0;
		p->core_handle_cycles = 0;

		snprintf(section_name, sizeof(section_name), "core %u", i);
		if(!rte_cfgfile_has_section(file, section_name))
		{
			rte_panic("Config file parse error: core IDs are not "
				"sequential (thread %u missing)\n", i);
			goto end;
		}

		entry = rte_cfgfile_get_entry(file, section_name, "type");
		if(!entry)
		{
			rte_panic("Config file parse error: core %u type not "
				"defined\n", i);
			goto end;
		}

		if((0 != udpi_core_type_string_to_id(entry, &p->core_type))
			|| (UDPI_CORE_NONE == p->core_type))
		{
			rte_panic("Config file parse error: core %u type "
				"error\n", i);
			goto end;
		}

		if (UDPI_CORE_IPV4_RX == p->core_type)
		{
			entry = rte_cfgfile_get_entry(file, section_name, "queue_id");
			if(!entry)
			{
				rte_panic("Config file parse error: core %u queue_id not "
				"defined\n", i);
				goto end;
			}
			p->id = strtoul(entry, NULL, 10);
			if (p->id >= UDPI_MAX_QUEUES)
			{
				rte_panic("Config file parse error: core %u queue_id "
					"error\n", i);
				goto end;
			}

			entry = rte_cfgfile_get_entry(file, section_name, "port_id");
			if(!entry)
			{
				rte_panic("Config file parse error: core %u port_id not "
				"defined\n", i);
				goto end;
			}
			p->port_id = strtoul(entry, NULL, 10);
			if (p->port_id >= udpi.n_ports)
			{
				rte_panic("Config file parse error: core %u port_id "
					"error\n", i);
				goto end;
			}
			udpi.ports[p->port_id].n_queues++;
			udpi.n_workers ++;
		}
	}

	for (i = 0; i < n_ports; i++)
	{
		struct udpi_port_params *p = &udpi.ports[i];
		char section_name[16];
		const char *entry = NULL;

		snprintf(section_name, sizeof(section_name), "port %u", i);
		if (!rte_cfgfile_has_section(file, section_name))
		{
			rte_panic("Config file parse error: port IDs are not "
				"sequential (port %u missing)\n", i);
			goto end;
		}

		entry = rte_cfgfile_get_entry(file, section_name, "type");
		if (!entry)
		{
			rte_panic("Config file parse error: core %u type not "
				"defined\n", i);
			goto end;
		}

		if ((0 != udpi_port_type_string_to_id(entry, &p->port_type))
			|| (UDPI_PORT_NONE == p->port_type))
		{
			rte_panic("Config file parse error: port %u type "
				"error\n", i);
			goto end;
		}
	}

	
	char section_name[16];
	const char *entry = NULL;
	snprintf(section_name, sizeof(section_name), "log");
	if(!rte_cfgfile_has_section(file, section_name))
	{
		rte_panic("Config file parse error: there is no log section)\n");
		goto end;
	}

	entry = rte_cfgfile_get_entry(file, section_name, "type");
	if(!entry)
	{
		rte_panic("Config file parse error: log type not defined\n");
		goto end;
	}

	if ((0 != udpi_log_type_string_to_level(entry, &udpi.log.level)))
	{
		rte_panic("Config file parse error: log type not correct\n");
		goto end;
	}
	

	ret = 0;
end:
	rte_cfgfile_close(file);
	
	return ret;
}

static uint64_t udpi_get_core_mask(void)
{
	uint64_t core_mask = 0;
	uint32_t i = 0;

	for(i = 0; i < RTE_MAX_LCORE; i++)
	{
		if(0 == rte_lcore_is_enabled(i))
		{
			continue;
		}

		core_mask |= (1LLU << i);
	}

	return core_mask;
}

static int
udpi_install_coremask(uint64_t core_mask)
{
	uint32_t n_cores, i;

	for (n_cores = 0, i = 0; i < RTE_MAX_LCORE; i++)
		if (udpi.cores[i].core_type != UDPI_CORE_NONE)
			n_cores++;

	if (n_cores != udpi.n_cores) {
		rte_panic("Number of cores in COREMASK should be %u instead "
			"of %u\n", n_cores, udpi.n_cores);
		return -1;
	}

	for (i = 0; i < RTE_MAX_LCORE; i++) {
		uint32_t core_id;

		if (udpi.cores[i].core_type == UDPI_CORE_NONE)
			continue;

		core_id = __builtin_ctzll(core_mask);
		core_mask &= ~(1LLU << core_id);

		udpi.cores[i].core_id = core_id;
	}

	return 0;
}

static void udpi_cores_config_print(void)
{
	uint32_t i = 0;

	for(i = 0; i < RTE_MAX_LCORE; i++)
	{
		struct udpi_core_params *p = &udpi.cores[i];

		if(UDPI_CORE_NONE == udpi.cores[i].core_type)
		{
			continue;
		}

		printf("core id = %u, core type = %6s", p->core_id, 
			udpi_core_type_id_to_string(p->core_type));
		if (UDPI_CORE_IPV4_RX == udpi.cores[i].core_type)
		{
			printf(", port_id = %u, queue_id = %u\n", p->port_id, p->id);
		}
		else
			printf("\n");
	}

	for(i = 0; i < udpi.n_ports; i++)
	{
		struct udpi_port_params *p = &udpi.ports[i];

		if(UDPI_PORT_NONE == udpi.ports[i].port_type)
		{
			continue;
		}
		printf("port id = %u, port type = %6s", p->port_id, 
			udpi_port_type_id_to_string(p->port_type));
	}

	return;
}

static int32_t udpi_install_portmask(const char *arg)
{
	char *end = NULL;
	uint64_t port_mask;
	uint32_t i;

	if (arg[0] == '\0')
		return -1;

	port_mask = strtoul(arg, &end, 16);
	if ((end == NULL) || (*end != '\0'))
		return -2;

	if (port_mask == 0)
		return -3;

	udpi.n_ports = 0;
	for (i = 0; i < 64; i++) {
		if ((port_mask & (1LLU << i)) == 0)
			continue;

		if (udpi.n_ports >= UDPI_MAX_PORTS)
			return -4;

		udpi.ports[udpi.n_ports].port_id = i;
		udpi.n_ports++;
	}

	if (!rte_is_power_of_2(udpi.n_ports))
		return -5;

	return 0;
}

int32_t udpi_parse_args(int argc, char **argv)
{
	int32_t ret = -1, opt = -1;
	int32_t option_index = -1;
	char **argvopt = NULL;
	char *prgname = argv[0];
	static struct option lgopts[] = {
		{NULL, 0, 0, 0}
	};
	uint64_t core_mask = udpi_get_core_mask();

	udpi.n_cores = __builtin_popcountll(core_mask);

	argvopt = argv;

	while((opt = getopt_long(argc, argvopt, "p:f:", 
		lgopts, &option_index)) != EOF)
	{
		switch(opt)
		{
			case 'p':
				if(0 != udpi_install_portmask(optarg))
				{
					rte_panic("PORT_ID should specify a number "
						"less than %u\n", UDPI_MAX_PORTS);
				}
				break;
			case 'f':
				udpi_install_cfgfile(optarg);				
				break;
			default:
				return -1;
		}
	}

	udpi_install_coremask(core_mask);
	udpi_cores_config_print();

	if(0 <= optind)
	{
		argv[optind - 1] = prgname;
	}

	ret = optind - 1;
	optind = 0;
	
	return ret;
}

