/*gcc -o main main.c  -lnl-route-3 -lnl-3  -I /usr/include/libnl3/ */
/*gcc -o main main.c   $(pkg-config --cflags --libs libnl-3.0 libnl-route-3.0)*/
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_tun.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <netlink/netlink.h>
#include <netlink/route/addr.h>
#include <netlink/route/link.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/rule.h>
#include <netlink/route/route.h>
#include <netlink/route/neighbour.h>

#define ROUTE_TABLE "/etc/iproute2/rt_tables"

#define NAME "eth0"
#define ETHER_ADDR_LEN  6

static void get_ip(struct nl_object *obj, void *arg)
{
	struct rtnl_addr * addr = (struct rtnl_addr *) obj;
	struct nl_addr *naddr = rtnl_addr_get_local(addr);
	int prefixlen = rtnl_addr_get_prefixlen(addr);
	if ((NULL != naddr) && (prefixlen == *(int*)(arg)))
		printf("ip is 0x%x\n", *(uint32_t *) (nl_addr_get_binary_addr(naddr)));
}

static void get_route(struct nl_object *obj, void *arg)
{
	struct rtnl_route *route = (struct rtnl_route *) obj;
	struct rtnl_nexthop * nexthop = rtnl_route_nexthop_n(route, 0);
	struct nl_addr *naddr = rtnl_route_nh_get_gateway(nexthop);

	if (NULL != naddr)
		printf("gwip is 0x%x\n", *(uint32_t *) (nl_addr_get_binary_addr(naddr)));
	
}

int main(int argc, char *argv[])
{
	struct nl_sock *nl_sock;
	struct nl_cache *link_cache;
	int ifindex;
	int ret = 0;
	int err = 0;

	if (argc < 2)
	{
		printf("%s ip gw on/off tip\n", argv[0]);
		return -1;
	}

//link
	if (err = rtnl_route_read_table_names(ROUTE_TABLE)) {
		printf("failed to read %s. err = %s\n", ROUTE_TABLE, nl_geterror(err));
		return -1;;
	}

	nl_sock = nl_socket_alloc();
	if (NULL == nl_sock) {
		printf("failed to alloc netlink handler.\n");
		return -1;
	}

	if (err = nl_connect(nl_sock, NETLINK_ROUTE)) {
		printf("failed to connect NETLINK_ROUTE. err = %s\n", nl_geterror(err));
		ret = -1;
		goto release_nl;
	}

	if (err = rtnl_link_alloc_cache(nl_sock, AF_INET, &link_cache))
	{
		printf("failed to allocate link cache. err = %s\n", nl_geterror(err));
		ret = -1;
		goto release_nl;
	}

	ifindex = rtnl_link_name2i(link_cache, NAME);
	if (0 == ifindex) {
		printf("%s - failed to find.\n", NAME);
		ret = -1;
		goto release_link_cache;
	}

	struct rtnl_link * link = rtnl_link_get(link_cache, ifindex);
	if (link == NULL)
	{
		printf("can't get link.\n");
		ret = -1;
		goto release_link_cache;
	}
	//rtnl_link_get_by_name

	struct nl_addr *lladdr = rtnl_link_get_addr(link);
	if (NULL == lladdr || AF_LLC != nl_addr_get_family(lladdr)) {
		printf("failed to get MAC\n");
		ret = -1;
		goto release_link;
	}

	uint8_t mac_address[ETHER_ADDR_LEN];
	memcpy(mac_address, nl_addr_get_binary_addr(lladdr), ETHER_ADDR_LEN);
	printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
		 mac_address[0],
		 mac_address[1],
		 mac_address[2],
		 mac_address[3],
		 mac_address[4],
		 mac_address[5]);

//addr 
	struct nl_cache * addr_cache;
	if (err  = rtnl_addr_alloc_cache(nl_sock, &addr_cache))
	{
		printf("fail to get addr_cache\n");
		ret = -1;
		goto release_link;
	}
	struct rtnl_addr *addr = rtnl_addr_alloc();
	rtnl_addr_set_ifindex(addr, ifindex);
	rtnl_addr_set_family(addr, AF_INET);
	int prefixlen = 16;
	nl_cache_foreach_filter(addr_cache, (struct nl_object *)addr, get_ip, &prefixlen);
	nl_cache_free(addr_cache);

	uint32_t ipaddr = inet_addr(argv[1]);
	struct nl_addr * local = nl_addr_build(AF_INET, &ipaddr, sizeof(ipaddr));
	rtnl_addr_set_local(addr, local);
	rtnl_addr_set_ifindex(addr, ifindex);
	rtnl_addr_set_family(addr, AF_INET);
	rtnl_addr_set_prefixlen(addr, 32);

	if (!strcmp(argv[2], "on"))
	{
		if (err = rtnl_addr_add(nl_sock, addr, 0))
		{
			printf("fail to add addr %s\n", nl_geterror(err));
			ret = -1;
			goto release_addr;
		}
	}
	else
	{
		if (err = rtnl_addr_delete(nl_sock, addr, 0))
		{
			printf("fail to del addr %s\n", nl_geterror(err));
			ret = -1;
			goto release_addr;
		}
		
	}
//neigh
	struct nl_cache * neigh_cache;
	if (err = rtnl_neigh_alloc_cache(nl_sock, &neigh_cache))
	{
		printf("failed to allocate neighbor cache. err = %s\n", nl_geterror(err));
		ret = -1;
		goto release_neigh_cache;
	}

	uint32_t gw = inet_addr(argv[3]);
	struct nl_addr * gw_addr = nl_addr_build(AF_INET, &gw, sizeof(gw));

	struct rtnl_neigh * neigh = rtnl_neigh_get(neigh_cache, ifindex, gw_addr);
	if (neigh) {
		// It's optional
		struct nl_addr * lladdr = rtnl_neigh_get_lladdr(neigh);
		if (lladdr) {
			uint8_t mac_address[ETHER_ADDR_LEN];
			memcpy(mac_address, nl_addr_get_binary_addr(lladdr), ETHER_ADDR_LEN);
			printf("gw  %02X:%02X:%02X:%02X:%02X:%02X\n",
		 		mac_address[0],
		 		mac_address[1],
		 		mac_address[2],
		 		mac_address[3],
		 		mac_address[4],
		 		mac_address[5]);
		}
	}

	nl_addr_put(gw_addr);

//route
	struct nl_cache *route_cache;
	if (err = rtnl_route_alloc_cache(nl_sock, AF_INET, 0, &route_cache))
	{
		printf("failed to allocate route cache. err = %s\n", nl_geterror(err));
		ret = -1;
		goto release_neigh_cache;
	}

	struct rtnl_route *route = rtnl_route_alloc();
	struct nl_addr * taddr;
	err = nl_addr_parse(argv[4], AF_INET, &taddr);
        if (err)
        {
                printf("failed to get taddr. err = %s\n", nl_geterror(err));
                ret = -1;
                goto release_route_cache;
                
        }

	nl_cache_foreach_filter(route_cache, OBJ_CAST(route), get_route, NULL);
/*
	struct nl_sock *nl_fib_sock;
	nl_fib_sock = nl_socket_alloc();
	if (err = nl_connect(nl_fib_sock, NETLINK_FIB_LOOKUP)) {
		printf("failed to connect NETLINK_ROUTE. err = %s\n", nl_geterror(err));
		ret = -1;
		goto release_nl;
	}
	struct nl_dump_params params = {
		.dp_fd = stdout,
		.dp_type = NL_DUMP_DETAILS,
	};

	struct nl_cache *route_cache = flnl_result_alloc_cache();

        struct flnl_request *req = flnl_request_alloc();
        struct nl_addr * taddr;
	err = nl_addr_parse(argv[4], AF_INET, &taddr);
	if (err)
	{
		printf("failed to get taddr. err = %s\n", nl_geterror(err));
		ret = -1;
		goto release_route;
		
	}
	int table = RT_TABLE_UNSPEC, scope = RT_SCOPE_UNIVERSE;
        flnl_request_set_addr(req, taddr);
	flnl_request_set_table(req, table);
	flnl_request_set_scope(req, scope);
        err = flnl_lookup(nl_fib_sock, req, route_cache);
	if (err)
	{
		printf("failed to fib lookup. err = %s\n", nl_geterror(err));
		ret = -1;
		goto release_route_addr;
	}
	
	nl_cache_dump(route_cache, &params);

release_route_addr:
	nl_addr_put(taddr);	

release_route:
        nl_cache_free(route_cache);
	nl_object_put(OBJ_CAST(req));

	nl_close(nl_fib_sock);
	nl_socket_free(nl_fib_sock);
*/
release_route_cache:
	nl_cache_free(route_cache);
release_neigh_cache:
	nl_cache_free(neigh_cache);
release_addr:
	nl_addr_put(local);
	rtnl_addr_put(addr);
release_link:
	rtnl_link_put(link);
release_link_cache:
	nl_cache_free(link_cache);
release_nl:
	nl_close(nl_sock);
	nl_socket_free(nl_sock);

	return ret;
}
