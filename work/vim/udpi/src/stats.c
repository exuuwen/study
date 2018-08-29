
#include "stats.h"
 
static void allstats_init(struct all_stats *stats)
{
	rte_atomic64_init(&stats->l3.ip_pkts);
	rte_atomic64_init(&stats->l3.ip_bytes);
	rte_atomic64_init(&stats->l4.udp_pkts);
	rte_atomic64_init(&stats->l4.udp_bytes);
	rte_atomic64_init(&stats->l4.tcp_pkts);
	rte_atomic64_init(&stats->l4.tcp_bytes);
	rte_atomic64_init(&stats->l4.tcp_syn_pkts);
	rte_atomic64_init(&stats->l4.tcp_retransmit_pkts);
	rte_atomic64_init(&stats->l4.tcp_retransmit_bytes);
	rte_atomic64_init(&stats->l7.ssh);
	rte_atomic64_init(&stats->l7.ssh_bytes);
	rte_atomic64_init(&stats->l7.http);
	rte_atomic64_init(&stats->l7.http_bytes);
	rte_atomic64_init(&stats->l7.https);
	rte_atomic64_init(&stats->l7.https_bytes);
	rte_atomic64_init(&stats->l7.others);
	rte_atomic64_init(&stats->l7.others_bytes);
}

static void connstats_init(struct connection_stats *stats)
{
	rte_atomic64_init(&stats->flow_entries);
	rte_atomic64_init(&stats->closed_flow_entries);
	rte_atomic64_init(&stats->tcp_rst_flow_entries);
	rte_atomic64_init(&stats->tcp_flow_entries);
	rte_atomic64_init(&stats->tcp_closed_flow_entries);
	rte_atomic64_init(&stats->udp_flow_entries);
	rte_atomic64_init(&stats->udp_closed_flow_entries);
}

static void compstats_init(struct comp_stats *stats)
{
	rte_atomic64_init(&stats->ip_bytes);
}

static void delyastats_init(struct delay_stats *stats)
{
	rte_atomic64_init(&stats->tcp_delay_ms);
	rte_atomic64_init(&stats->tcp_delay_flows);
}

static void mapstats_init(struct map_stats *stats)
{
	allstats_init(&stats->input_stats);
	allstats_init(&stats->output_stats);
	connstats_init(&stats->flows_stats);
	delyastats_init(&stats->delay_stats);
}

void udpi_stats_init(struct udpi_stats *stats)
{
	allstats_init(&stats->input_stats);
	allstats_init(&stats->output_stats);
	connstats_init(&stats->flows_stats);
	delyastats_init(&stats->delay_stats);

	int i, j;
	for (i=0; i<UDPI_COMP_MAX; i++)
	{
		compstats_init(&stats->input_comp_stats[i]);
		compstats_init(&stats->output_comp_stats[i]);
	}

	for (i=0; i<UDPI_REGION_STATS; i++)
		for (j=0; j<UDPI_OPERATOR_STATS; j++)
			mapstats_init(&stats->map_stats[i][j]);

	int k =0;
	for (k=0; k<UDPI_LINE_STATS; k++)
	{
		allstats_init(&stats->line_stats[k].input_stats);
		allstats_init(&stats->line_stats[k].output_stats);
		connstats_init(&stats->line_stats[k].flows_stats);
		delyastats_init(&stats->line_stats[k].delay_stats);


		for (i=0; i<UDPI_REGION_STATS; i++)
		for(j=0; j<UDPI_OPERATOR_STATS; j++)
		{
			if (stats->line_stats[k].map_stats[i][j].seen)
			{
				mapstats_init(&stats->line_stats[k].map_stats[i][j]);
			
			}
		}
	}

}

/*
void udpi_eipstats_init(struct udpi_eip_stats *stats)
{
	allstats_init(&stats->input_stats);
	allstats_init(&stats->output_stats);
	connstats_init(&stats->flows_stats);
	delyastats_init(&stats->delay_stats);

	int i, j;
	for (i=0; i<UDPI_REGION_STATS; i++)
		for (j=0; j<UDPI_OPERATOR_STATS; j++)
			mapstats_init(&stats->map_stats[i][j]);
}
*/

