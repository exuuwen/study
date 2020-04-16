/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <config.h>
#include "traceroute-switch.h"

#include <errno.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <time.h>
#include <linux/if_ether.h>
#include <linux/icmp.h>
#include <linux/ip.h>

//#include "dp-packet.h"
#include "openflow/openflow.h"
#include "openflow/nicira-ext.h"
#include "openvswitch/ofp-actions.h"
#include "openvswitch/ofp-errors.h"
#include "openvswitch/ofp-msgs.h"
#include "openvswitch/ofp-print.h"
#include "openvswitch/ofp-util.h"
#include "openvswitch/ofp-parse.h"
#include "openvswitch/ofpbuf.h"
#include "openvswitch/vconn.h"
#include "openvswitch/vlog.h"
#include "rconn.h"
#include "random.h"
#include "csum.h"

VLOG_DEFINE_THIS_MODULE(learning_switch);

enum lswitch_state {
    S_CONNECTING,               /* Waiting for connection to complete. */
    S_FEATURES_REPLY,           /* Waiting for features reply. */
    S_SWITCHING,                /* Switching flows. */
};

struct lswitch {
    struct rconn *rconn;
    enum lswitch_state state;

    /* If nonnegative, the switch sets up flows that expire after the given
     * number of seconds (or never expire, if the value is OFP_FLOW_PERMANENT).
     * Otherwise, the switch processes every packet. */
    int max_idle;

    enum ofputil_protocol protocol;
    unsigned long long int datapath_id;
    struct flow_wildcards wc;   /* Wildcards to apply to flows. */

    /* Queue distribution. */
    uint32_t default_queue;     /* Default OpenFlow queue, or UINT32_MAX. */

    /* Number of outgoing queued packets on the rconn. */
    struct rconn_packet_counter *queued;

    /* Optional "flow mod" requests to send to the switch at connection time,
     * to set up the flow table. */
    const struct ofputil_flow_mod *default_flows;
    size_t n_default_flows;
    enum ofputil_protocol usable_protocols;
};

/* The log messages here could actually be useful in debugging, so keep the
 * rate limit relatively high. */
static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(30, 300);

static void queue_tx(struct lswitch *, struct ofpbuf *);
static void send_features_request(struct lswitch *);

static void lswitch_process_packet(struct lswitch *, const struct ofpbuf *);
static enum ofperr process_switch_features(struct lswitch *,
                                           struct ofp_header *);
static void process_packet_in(struct lswitch *, const struct ofp_header *);
static void process_echo_request(struct lswitch *, const struct ofp_header *);

/* Creates and returns a new learning switch whose configuration is given by
 * 'cfg'.
 *
 * 'rconn' is used to send out an OpenFlow features request. */
struct lswitch *
lswitch_create(struct rconn *rconn, const struct lswitch_config *cfg)
{
    struct lswitch *sw;

    sw = xzalloc(sizeof *sw);
    sw->rconn = rconn;
    sw->state = S_CONNECTING;
    sw->max_idle = cfg->max_idle;
    sw->datapath_id = 0;

    sw->default_flows = cfg->default_flows;
    sw->n_default_flows = cfg->n_default_flows;
    sw->usable_protocols = cfg->usable_protocols;
    sw->queued = rconn_packet_counter_create();

    VLOG_INFO("new lswitch in");

    return sw;
}

static void
lswitch_handshake(struct lswitch *sw)
{
    enum ofputil_protocol protocol;
    enum ofp_version version;

    send_features_request(sw);

    version = rconn_get_version(sw->rconn);
    protocol = ofputil_protocol_from_ofp_version(version);
    
    if (sw->default_flows) {
        struct ofpbuf *msg = NULL;
        int error = 0;
        size_t i;

        if (!(protocol & sw->usable_protocols)) {
            enum ofputil_protocol want = rightmost_1bit(sw->usable_protocols);
            while (!error) {
                msg = ofputil_encode_set_protocol(protocol, want, &protocol);
                if (!msg) {
                    break;
                }
                error = rconn_send(sw->rconn, msg, NULL);
            }
        }
        if (protocol & sw->usable_protocols) {
            for (i = 0; !error && i < sw->n_default_flows; i++) {
                msg = ofputil_encode_flow_mod(&sw->default_flows[i], protocol);
                error = rconn_send(sw->rconn, msg, NULL);
            }

            if (error) {
                VLOG_INFO_RL(&rl, "%s: failed to queue default flows (%s)",
                             rconn_get_name(sw->rconn), ovs_strerror(error));
            }
        } else {
            VLOG_INFO_RL(&rl, "%s: failed to set usable protocol",
                         rconn_get_name(sw->rconn));
        }
    }
    sw->protocol = protocol;
}

bool
lswitch_is_alive(const struct lswitch *sw)
{
    return rconn_is_alive(sw->rconn);
}

/* Destroys 'sw'. */
void
lswitch_destroy(struct lswitch *sw)
{
    if (sw) {
        rconn_destroy(sw->rconn);
        rconn_packet_counter_destroy(sw->queued);
        free(sw);
    }
}

/* Takes care of necessary 'sw' activity, except for receiving packets (which
 * the caller must do). */
void
lswitch_run(struct lswitch *sw)
{
    int i;

    rconn_run(sw->rconn);

    if (sw->state == S_CONNECTING) {
        if (rconn_get_version(sw->rconn) != -1) {
            lswitch_handshake(sw);
            sw->state = S_FEATURES_REPLY;
        }
        return;
    }

    for (i = 0; i < 50; i++) {
        struct ofpbuf *msg;

        msg = rconn_recv(sw->rconn);
        if (!msg) {
            break;
        }

        lswitch_process_packet(sw, msg);
        ofpbuf_delete(msg);
    }
}

void
lswitch_wait(struct lswitch *sw)
{
    rconn_run_wait(sw->rconn);
    rconn_recv_wait(sw->rconn);
}

/* Processes 'msg', which should be an OpenFlow received on 'rconn', according
 * to the learning switch state in 'sw'.  The most likely result of processing
 * is that flow-setup and packet-out OpenFlow messages will be sent out on
 * 'rconn'.  */
static void
lswitch_process_packet(struct lswitch *sw, const struct ofpbuf *msg)
{
    enum ofptype type;
    struct ofpbuf b;

    b = *msg;
    if (ofptype_pull(&type, &b)) {
        return;
    }

    if (sw->state == S_FEATURES_REPLY
        && type != OFPTYPE_ECHO_REQUEST
        && type != OFPTYPE_FEATURES_REPLY) {
        return;
    }

    if (type == OFPTYPE_ECHO_REQUEST) {
        process_echo_request(sw, msg->data);
    } else if (type == OFPTYPE_FEATURES_REPLY) {
        if (sw->state == S_FEATURES_REPLY) {
            if (!process_switch_features(sw, msg->data)) {
                sw->state = S_SWITCHING;
            } else {
                rconn_disconnect(sw->rconn);
            }
        }
    } else if (type == OFPTYPE_PACKET_IN) {
        process_packet_in(sw, msg->data);
    } else if (type == OFPTYPE_FLOW_REMOVED) {
        /* Nothing to do. */
    } else if (VLOG_IS_DBG_ENABLED()) {
        char *s = ofp_to_string(msg->data, msg->size, 2);
        VLOG_DBG_RL(&rl, "%016llx: OpenFlow packet ignored: %s",
                    sw->datapath_id, s);
        free(s);
    }
}

static void
send_features_request(struct lswitch *sw)
{
    struct ofpbuf *b;
    int ofp_version = rconn_get_version(sw->rconn);

    ovs_assert(ofp_version > 0 && ofp_version < 0xff);

    /* Send OFPT_FEATURES_REQUEST. */
    b = ofpraw_alloc(OFPRAW_OFPT_FEATURES_REQUEST, ofp_version, 0);
    queue_tx(sw, b);

    /* Send OFPT_SET_CONFIG. */
    struct ofputil_switch_config config = {
        .miss_send_len = OFP_DEFAULT_MISS_SEND_LEN
    };
    queue_tx(sw, ofputil_encode_set_config(&config, ofp_version));
}

static void
queue_tx(struct lswitch *sw, struct ofpbuf *b)
{
    int retval = rconn_send_with_limit(sw->rconn, b, sw->queued, 10);
    if (retval && retval != ENOTCONN) {
        if (retval == EAGAIN) {
            VLOG_INFO_RL(&rl, "%016llx: %s: tx queue overflow",
                         sw->datapath_id, rconn_get_name(sw->rconn));
        } else {
            VLOG_WARN_RL(&rl, "%016llx: %s: send: %s",
                         sw->datapath_id, rconn_get_name(sw->rconn),
                         ovs_strerror(retval));
        }
    }
}

static enum ofperr
process_switch_features(struct lswitch *sw, struct ofp_header *oh)
{
    struct ofputil_switch_features features;

    struct ofpbuf b = ofpbuf_const_initializer(oh, ntohs(oh->length));
    enum ofperr error = ofputil_pull_switch_features(&b, &features);
    if (error) {
        VLOG_ERR("received invalid switch feature reply (%s)",
                 ofperr_to_string(error));
        return error;
    }

    sw->datapath_id = features.datapath_id;

    return 0;
}

static uint16_t ip_ident_seed = 100;
static inline uint16_t icmp_cksum(uint16_t l4_len,
                                          struct icmphdr const* icmp) 
{
    uint16_t cksum;

    cksum = csum((void*)icmp, l4_len);
    return cksum;
}

static inline uint16_t
ipv4_cksum(const struct iphdr *ipv4_hdr)
{
    uint16_t cksum;
    
    cksum = csum((void*)ipv4_hdr, sizeof(struct iphdr));
    return cksum;
}

extern struct in_addr saddr;

static void icmp_unexpect_ttl(uint8_t *icmp_packet, struct ofputil_packet_in *pi, uint8_t header_len, uint8_t type, uint8_t code, uint32_t info)
{
    uint16_t total_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + header_len + 8;
    uint8_t *old_payload = pi->packet;

    struct ethhdr * old_eth = (struct ethhdr *)old_payload;
    struct ethhdr * new_eth = (struct ethhdr *)icmp_packet;
    memcpy(new_eth->h_source, old_eth->h_dest, sizeof(struct eth_addr));
    memcpy(new_eth->h_dest, old_eth->h_source, sizeof(struct eth_addr));
    new_eth->h_proto = htons(ETH_P_IP);

    struct iphdr * old_ipv4 = (struct iphdr *) (old_eth + 1);
    struct iphdr * new_ipv4 = (struct iphdr *) (new_eth + 1);
    new_ipv4->ihl = 0x5;
    new_ipv4->version = 0x4;
    new_ipv4->tos = old_ipv4->tos;
    new_ipv4->tot_len = htons(total_len);
    ip_ident_seed += 27;
    new_ipv4->id = htons(ip_ident_seed);
    new_ipv4->frag_off = 0;
    new_ipv4->ttl = 64;
    new_ipv4->protocol = IPPROTO_ICMP;
    new_ipv4->check = 0;
    new_ipv4->saddr = saddr.s_addr;
    new_ipv4->daddr = old_ipv4->saddr;
    new_ipv4->check = ipv4_cksum(new_ipv4);

    struct icmphdr * icmp = (struct icmphdr *) (new_ipv4 + 1);
    icmp->type = type;
    icmp->code = code;
    icmp->un.gateway = info;
    icmp->checksum = 0;

    uint8_t *payload = (uint8_t *)(icmp + 1);
    old_payload += sizeof(struct ethhdr);
    memcpy(payload, old_payload, 8 + header_len);

    icmp->checksum = icmp_cksum(total_len - sizeof(struct iphdr), (struct icmphdr const*) icmp);

    return;
}

static void
process_packet_in(struct lswitch *sw, const struct ofp_header *oh)
{
    struct ofputil_packet_in pi;
    uint32_t buffer_id;

    uint64_t ofpacts_stub[64];
    struct ofpbuf ofpacts;

    struct ofputil_packet_out po;
    enum ofperr error;
    uint8_t icmp_packet[128];

    const struct mf_field *mf; 
    const struct mf_field *mf_id; 
    union mf_value sf_value, sf_mask;
    union mf_value sf_value_id, sf_mask_id;

    error = ofputil_decode_packet_in(oh, true, &pi, NULL, &buffer_id, NULL);
    if (error) {
        VLOG_WARN_RL(&rl, "failed to decode packet-in: %s",
                     ofperr_to_string(error));
        return;
    }

    /* Ignore packets sent via output to OFPP_CONTROLLER.  This library never
     * uses such an action.  You never know what experiments might be going on,
     * though, and it seems best not to interfere with them. */
    if (pi.reason != OFPR_ACTION || buffer_id != UINT32_MAX) {
        return;
    }

    struct ethhdr *old_pkt = pi.packet;
   
    struct iphdr * old_ipv4 = (struct iphdr*)(old_pkt + 1);
    uint16_t old_tot_len = ntohs(old_ipv4->tot_len);
    uint8_t header_len = old_ipv4->ihl*4;

    if (pi.packet_len < header_len + sizeof(struct ethhdr) + 8 || old_tot_len < header_len + 8)
        return;

    if (old_pkt->h_proto != htons(ETH_P_IP))
        return;

    //VLOG_INFO("AFTER MATCH");

    uint32_t icmp_packet_len = sizeof(struct ethhdr) + sizeof(struct iphdr) + header_len + sizeof(struct icmphdr) + 8;

    icmp_unexpect_ttl(icmp_packet, &pi, header_len, ICMP_TIME_EXCEEDED, ICMP_EXC_TTL, 0);

    ofpbuf_use_stack(&ofpacts, ofpacts_stub, sizeof ofpacts_stub);

    mf_id = mf_from_name("tun_id");
    if (!mf_id)
        return;

    sf_value_id.be64 = pi.flow_metadata.flow.tunnel.tun_id;
    sf_mask_id.be64 = OVS_BE64_MAX;
    ofpact_put_set_field(&ofpacts, mf_id, &sf_value_id, &sf_mask_id);

    mf = mf_from_name("tun_dst");
    if (!mf)
        return;

    sf_value.be32 = pi.flow_metadata.flow.tunnel.ip_src;
    sf_mask.be32 = OVS_BE32_MAX;
    ofpact_put_set_field(&ofpacts, mf, &sf_value, &sf_mask);

    ofpact_put_OUTPUT(&ofpacts)->port = OFPP_IN_PORT;

    /* Prepare packet_out in case we need one. */
    po.buffer_id = buffer_id;
    po.packet = icmp_packet;
    po.packet_len = icmp_packet_len;

    po.in_port = pi.flow_metadata.flow.in_port.ofp_port;
    po.ofpacts = ofpacts.data;
    po.ofpacts_len = ofpacts.size;


    queue_tx(sw, ofputil_encode_packet_out(&po, sw->protocol));

    //VLOG_INFO("AFTER MATCH last");
}

static void
process_echo_request(struct lswitch *sw, const struct ofp_header *rq)
{
    queue_tx(sw, make_echo_reply(rq));
}
