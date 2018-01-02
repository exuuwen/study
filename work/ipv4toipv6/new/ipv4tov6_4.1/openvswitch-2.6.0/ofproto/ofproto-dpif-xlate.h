/* Copyright (c) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Nicira, Inc.
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
 * limitations under the License. */

#ifndef OFPROTO_DPIF_XLATE_H
#define OFPROTO_DPIF_XLATE_H 1

#include "dp-packet.h"
#include "flow.h"
#include "openvswitch/meta-flow.h"
#include "odp-util.h"
#include "openvswitch/ofpbuf.h"
#include "ofproto-dpif-mirror.h"
#include "ofproto-dpif-rid.h"
#include "ofproto-dpif.h"
#include "ofproto.h"
#include "stp.h"
#include "ovs-lldp.h"

struct bfd;
struct bond;
struct dpif;
struct lacp;
struct dpif_ipfix;
struct dpif_sflow;
struct mac_learning;
struct mcast_snooping;
struct xlate_cache;

struct xlate_out {
    enum slow_path_reason slow; /* 0 if fast path may be used. */

    struct recirc_refs recircs; /* Recirc action IDs on which references are
                                 * held. */
};

struct xlate_in {
    struct ofproto_dpif *ofproto;

    /* Flow to which the OpenFlow actions apply.  xlate_actions() will modify
     * this flow when actions change header fields. */
    struct flow flow;

    /* The packet corresponding to 'flow', or a null pointer if we are
     * revalidating without a packet to refer to. */
    const struct dp_packet *packet;

    /* Should OFPP_NORMAL update the MAC learning table?  Should "learn"
     * actions update the flow table?
     *
     * We want to update these tables if we are actually processing a packet,
     * or if we are accounting for packets that the datapath has processed, but
     * not if we are just revalidating. */
    bool may_learn;

    /* The rule initiating translation or NULL. If both 'rule' and 'ofpacts'
     * are NULL, xlate_actions() will do the initial rule lookup itself. */
    struct rule_dpif *rule;

    /* The actions to translate.  If 'rule' is not NULL, these may be NULL. */
    const struct ofpact *ofpacts;
    size_t ofpacts_len;

    /* Union of the set of TCP flags seen so far in this flow.  (Used only by
     * NXAST_FIN_TIMEOUT.  Set to zero to avoid updating updating rules'
     * timeouts.) */
    uint16_t tcp_flags;

    /* If nonnull, flow translation calls this function just before executing a
     * resubmit or OFPP_TABLE action.  In addition, disables logging of traces
     * when the recursion depth is exceeded.
     *
     * 'rule' is the rule being submitted into.  It will be null if the
     * resubmit or OFPP_TABLE action didn't find a matching rule.
     *
     * 'indentation' is the resubmit recursion depth at time of invocation,
     * suitable for indenting the output.
     *
     * This is normally null so the client has to set it manually after
     * calling xlate_in_init(). */
    void (*resubmit_hook)(struct xlate_in *, struct rule_dpif *rule,
                          int indentation);

    /* If nonnull, flow translation calls this function to report some
     * significant decision, e.g. to explain why OFPP_NORMAL translation
     * dropped a packet.  'indentation' is the resubmit recursion depth at time
     * of invocation, suitable for indenting the output. */
    void (*report_hook)(struct xlate_in *, int indentation,
                        const char *format, va_list args);

    /* If nonnull, flow translation credits the specified statistics to each
     * rule reached through a resubmit or OFPP_TABLE action.
     *
     * This is normally null so the client has to set it manually after
     * calling xlate_in_init(). */
    const struct dpif_flow_stats *resubmit_stats;

    /* Counters carried over from a pre-existing translation of a related flow.
     * This can occur due to, e.g., the translation of an ARP packet that was
     * generated as the result of outputting to a tunnel port.  In that case,
     * the original flow going to the tunnel is the related flow.  Since the
     * two flows are different, they should not use the same xlate_ctx
     * structure.  However, we still need limit the maximum recursion across
     * the entire translation.
     *
     * These fields are normally set to zero, so the client has to set them
     * manually after calling xlate_in_init().  In that case, they should be
     * copied from the same-named fields in the related flow's xlate_ctx.
     *
     * These fields are really implementation details; the client doesn't care
     * about what they mean.  See the corresponding fields in xlate_ctx for
     * real documentation. */
    int indentation;
    int depth;
    int resubmits;

    /* If nonnull, flow translation populates this cache with references to all
     * modules that are affected by translation. This 'xlate_cache' may be
     * passed to xlate_push_stats() to perform the same function as
     * xlate_actions() without the full cost of translation.
     *
     * This is normally null so the client has to set it manually after
     * calling xlate_in_init(). */
    struct xlate_cache *xcache;

    /* If nonnull, flow translation puts the resulting datapath actions in this
     * buffer.  If null, flow translation will not produce datapath actions. */
    struct ofpbuf *odp_actions;

    /* If nonnull, flow translation populates this with wildcards relevant in
     * translation.  Any fields that were used to calculate the action are set,
     * to allow caching and kernel wildcarding to work.  For example, if the
     * flow lookup involved performing the "normal" action on IPv4 and ARP
     * packets, 'wc' would have the 'in_port' (always set), 'dl_type' (flow
     * match), 'vlan_tci' (normal action), and 'dl_dst' (normal action) fields
     * set. */
    struct flow_wildcards *wc;

    /* The frozen state to be resumed, as returned by xlate_lookup(). */
    const struct frozen_state *frozen_state;
};

void xlate_ofproto_set(struct ofproto_dpif *, const char *name, struct dpif *,
                       const struct mac_learning *, struct stp *,
                       struct rstp *, const struct mcast_snooping *,
                       const struct mbridge *, const struct dpif_sflow *,
                       const struct dpif_ipfix *, const struct netflow *,
                       bool forward_bpdu, bool has_in_band,
                       const struct dpif_backer_support *support);
void xlate_remove_ofproto(struct ofproto_dpif *);

void xlate_bundle_set(struct ofproto_dpif *, struct ofbundle *,
                      const char *name, enum port_vlan_mode, int vlan,
                      unsigned long *trunks, bool use_priority_tags,
                      const struct bond *, const struct lacp *,
                      bool floodable);
void xlate_bundle_remove(struct ofbundle *);

void xlate_ofport_set(struct ofproto_dpif *, struct ofbundle *,
                      struct ofport_dpif *, ofp_port_t, odp_port_t,
                      const struct netdev *, const struct cfm *, const struct bfd *,
                      const struct lldp *, struct ofport_dpif *peer,
                      int stp_port_no, const struct rstp_port *rstp_port,
                      const struct ofproto_port_queue *qdscp,
                      size_t n_qdscp, enum ofputil_port_config,
                      enum ofputil_port_state, bool is_tunnel,
                      bool may_enable);
void xlate_ofport_remove(struct ofport_dpif *);

struct ofproto_dpif * xlate_lookup_ofproto(const struct dpif_backer *,
                                           const struct flow *,
                                           ofp_port_t *ofp_in_port);
int xlate_lookup(const struct dpif_backer *, const struct flow *,
                 struct ofproto_dpif **, struct dpif_ipfix **,
                 struct dpif_sflow **, struct netflow **,
                 ofp_port_t *ofp_in_port);

enum xlate_error {
    XLATE_OK = 0,
    XLATE_BRIDGE_NOT_FOUND,
    XLATE_RECURSION_TOO_DEEP,
    XLATE_TOO_MANY_RESUBMITS,
    XLATE_STACK_TOO_DEEP,
    XLATE_NO_RECIRCULATION_CONTEXT,
    XLATE_RECIRCULATION_CONFLICT,
    XLATE_TOO_MANY_MPLS_LABELS,
};

const char *xlate_strerror(enum xlate_error error);

enum xlate_error xlate_actions(struct xlate_in *, struct xlate_out *);

void xlate_in_init(struct xlate_in *, struct ofproto_dpif *,
                   const struct flow *, ofp_port_t in_port, struct rule_dpif *,
                   uint16_t tcp_flags, const struct dp_packet *packet,
                   struct flow_wildcards *, struct ofpbuf *odp_actions);
void xlate_out_uninit(struct xlate_out *);
void xlate_actions_for_side_effects(struct xlate_in *);

enum ofperr xlate_resume(struct ofproto_dpif *,
                         const struct ofputil_packet_in_private *,
                         struct ofpbuf *odp_actions, enum slow_path_reason *);

int xlate_send_packet(const struct ofport_dpif *, bool oam, struct dp_packet *);

struct xlate_cache *xlate_cache_new(void);
void xlate_push_stats(struct xlate_cache *, const struct dpif_flow_stats *);
void xlate_cache_clear(struct xlate_cache *);
void xlate_cache_delete(struct xlate_cache *);

void xlate_txn_start(void);
void xlate_txn_commit(void);

#endif /* ofproto-dpif-xlate.h */
