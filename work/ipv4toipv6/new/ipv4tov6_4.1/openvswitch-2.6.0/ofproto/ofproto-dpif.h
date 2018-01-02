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

#ifndef OFPROTO_DPIF_H
#define OFPROTO_DPIF_H 1

#include <stdint.h>

#include "fail-open.h"
#include "hmapx.h"
#include "odp-util.h"
#include "openvswitch/ofp-util.h"
#include "ovs-thread.h"
#include "ofproto-provider.h"
#include "timer.h"
#include "util.h"
#include "ovs-thread.h"

/* Priority for internal rules created to handle recirculation */
#define RECIRC_RULE_PRIORITY 20

union user_action_cookie;
struct dpif_flow_stats;
struct ofproto;
struct ofproto_async_msg;
struct ofproto_dpif;
struct ofport_dpif;
struct dpif_backer;
struct OVS_LOCKABLE rule_dpif;
struct OVS_LOCKABLE group_dpif;

/* Number of implemented OpenFlow tables. */
enum { N_TABLES = 255 };
enum { TBL_INTERNAL = N_TABLES - 1 };    /* Used for internal hidden rules. */
BUILD_ASSERT_DECL(N_TABLES >= 2 && N_TABLES <= 255);

/* Ofproto-dpif -- DPIF based ofproto implementation.
 *
 * Ofproto-dpif provides an ofproto implementation for those platforms which
 * implement the netdev and dpif interface defined in netdev.h and dpif.h.  The
 * most important of which is the Linux Kernel Module (dpif-linux), but
 * alternatives are supported such as a userspace only implementation
 * (dpif-netdev), and a dummy implementation used for unit testing.
 *
 * Ofproto-dpif is divided into three major chunks.
 *
 * - ofproto-dpif.c
 *   The main ofproto-dpif module is responsible for implementing the
 *   provider interface, installing and removing datapath flows, maintaining
 *   packet statistics, running protocols (BFD, LACP, STP, etc), and
 *   configuring relevant submodules.
 *
 * - ofproto-dpif-upcall.c
 *   Ofproto-dpif-upcall is responsible for retrieving upcalls from the kernel,
 *   processing miss upcalls, and handing more complex ones up to the main
 *   ofproto-dpif module.  Miss upcall processing boils down to figuring out
 *   what each packet's actions are, executing them (i.e. asking the kernel to
 *   forward it), and handing it up to ofproto-dpif to decided whether or not
 *   to install a kernel flow.
 *
 * - ofproto-dpif-xlate.c
 *   Ofproto-dpif-xlate is responsible for translating OpenFlow actions into
 *   datapath actions. */

/* Stores the various features which the corresponding backer supports. */
struct dpif_backer_support {
    /* True if the datapath supports variable-length
     * OVS_USERSPACE_ATTR_USERDATA in OVS_ACTION_ATTR_USERSPACE actions.
     * False if the datapath supports only 8-byte (or shorter) userdata. */
    bool variable_length_userdata;

    /* True if the datapath supports masked data in OVS_ACTION_ATTR_SET
     * actions. */
    bool masked_set_action;

    /* True if the datapath supports tnl_push and pop actions. */
    bool tnl_push_pop;

    /* True if the datapath supports OVS_FLOW_ATTR_UFID. */
    bool ufid;

    /* True if the datapath supports OVS_ACTION_ATTR_TRUNC action. */
    bool trunc;

    /* Each member represents support for related OVS_KEY_ATTR_* fields. */
    struct odp_support odp;
};

bool ofproto_dpif_get_enable_ufid(const struct dpif_backer *backer);
struct dpif_backer_support *ofproto_dpif_get_support(const struct ofproto_dpif *);

ovs_version_t ofproto_dpif_get_tables_version(struct ofproto_dpif *);

struct rule_dpif *rule_dpif_lookup_from_table(struct ofproto_dpif *,
                                              ovs_version_t, struct flow *,
                                              struct flow_wildcards *,
                                              const struct dpif_flow_stats *,
                                              uint8_t *table_id,
                                              ofp_port_t in_port,
                                              bool may_packet_in,
                                              bool honor_table_miss);

static inline void rule_dpif_ref(struct rule_dpif *);
static inline void rule_dpif_unref(struct rule_dpif *);

void rule_dpif_credit_stats(struct rule_dpif *rule ,
                            const struct dpif_flow_stats *);

static inline bool rule_dpif_is_fail_open(const struct rule_dpif *);
static inline bool rule_dpif_is_table_miss(const struct rule_dpif *);
static inline bool rule_dpif_is_internal(const struct rule_dpif *);

uint8_t rule_dpif_get_table(const struct rule_dpif *);

bool table_is_internal(uint8_t table_id);

const struct rule_actions *rule_dpif_get_actions(const struct rule_dpif *);
void rule_set_recirc_id(struct rule *, uint32_t id);

ovs_be64 rule_dpif_get_flow_cookie(const struct rule_dpif *rule);

void rule_dpif_reduce_timeouts(struct rule_dpif *rule, uint16_t idle_timeout,
                               uint16_t hard_timeout);

void group_dpif_credit_stats(struct group_dpif *,
                             struct ofputil_bucket *,
                             const struct dpif_flow_stats *);
struct group_dpif *group_dpif_lookup(struct ofproto_dpif *ofproto,
                                     uint32_t group_id, ovs_version_t version,
                                     bool take_ref);
const struct ovs_list *group_dpif_get_buckets(const struct group_dpif *group);

enum ofp11_group_type group_dpif_get_type(const struct group_dpif *group);
const char *group_dpif_get_selection_method(const struct group_dpif *group);
uint64_t group_dpif_get_selection_method_param(const struct group_dpif *group);
const struct field_array *group_dpif_get_fields(const struct group_dpif *group);

int ofproto_dpif_execute_actions(struct ofproto_dpif *, const struct flow *,
                                 struct rule_dpif *, const struct ofpact *,
                                 size_t ofpacts_len, struct dp_packet *);
int ofproto_dpif_execute_actions__(struct ofproto_dpif *, const struct flow *,
                                   struct rule_dpif *, const struct ofpact *,
                                   size_t ofpacts_len, int indentation,
                                   int depth, int resubmits,
                                   struct dp_packet *);
void ofproto_dpif_send_async_msg(struct ofproto_dpif *,
                                 struct ofproto_async_msg *);
bool ofproto_dpif_wants_packet_in_on_miss(struct ofproto_dpif *);
int ofproto_dpif_send_packet(const struct ofport_dpif *, bool oam,
                             struct dp_packet *);
void ofproto_dpif_flow_mod(struct ofproto_dpif *,
                           const struct ofputil_flow_mod *);
struct rule_dpif *ofproto_dpif_refresh_rule(struct rule_dpif *);

struct ofport_dpif *odp_port_to_ofport(const struct dpif_backer *, odp_port_t);
struct ofport_dpif *ofp_port_to_ofport(const struct ofproto_dpif *,
                                       ofp_port_t);

bool ofproto_dpif_backer_enabled(struct dpif_backer* backer);

int ofproto_dpif_add_internal_flow(struct ofproto_dpif *,
                                   const struct match *, int priority,
                                   uint16_t idle_timeout,
                                   const struct ofpbuf *ofpacts,
                                   struct rule **rulep);
int ofproto_dpif_delete_internal_flow(struct ofproto_dpif *, struct match *,
                                      int priority);

const struct uuid *ofproto_dpif_get_uuid(const struct ofproto_dpif *);

/* struct rule_dpif has struct rule as it's first member. */
#define RULE_CAST(RULE) ((struct rule *)RULE)
#define GROUP_CAST(GROUP) ((struct ofgroup *)GROUP)

static inline struct group_dpif* group_dpif_ref(struct group_dpif *group)
{
    if (group) {
        ofproto_group_ref(GROUP_CAST(group));
    }
    return group;
}

static inline void group_dpif_unref(struct group_dpif *group)
{
    if (group) {
        ofproto_group_unref(GROUP_CAST(group));
    }
}

static inline void rule_dpif_ref(struct rule_dpif *rule)
{
    if (rule) {
        ofproto_rule_ref(RULE_CAST(rule));
    }
}

static inline void rule_dpif_unref(struct rule_dpif *rule)
{
    if (rule) {
        ofproto_rule_unref(RULE_CAST(rule));
    }
}

static inline bool rule_dpif_is_fail_open(const struct rule_dpif *rule)
{
    return is_fail_open_rule(RULE_CAST(rule));
}

static inline bool rule_dpif_is_table_miss(const struct rule_dpif *rule)
{
    return rule_is_table_miss(RULE_CAST(rule));
}

/* Returns true if 'rule' is an internal rule, false otherwise. */
static inline bool rule_dpif_is_internal(const struct rule_dpif *rule)
{
    return RULE_CAST(rule)->table_id == TBL_INTERNAL;
}

#undef RULE_CAST

bool ovs_native_tunneling_is_on(struct ofproto_dpif *ofproto);
#endif /* ofproto-dpif.h */
