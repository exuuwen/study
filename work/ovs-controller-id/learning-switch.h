/*
 * Copyright (c) 2008, 2010, 2011, 2012 Nicira, Inc.
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

#ifndef LEARNING_SWITCH_H
#define LEARNING_SWITCH_H 1

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "openvswitch/ofp-util.h"

struct ofpbuf;
struct rconn;

enum lswitch_mode {
    LSW_NORMAL,                 /* Always use OFPP_NORMAL. */
    LSW_FLOOD,                  /* Always use OFPP_FLOOD. */
    LSW_LEARN                   /* Learn MACs at controller. */
};

struct lswitch_config {
    enum lswitch_mode mode;

    /* <0: Process every packet at the controller.
     * >=0: Expire flows after they are unused for 'max_idle' seconds.
     * OFP_FLOW_PERMANENT: Set up permanent flows. */
    int max_idle;

    /* Optional "flow mod" requests to send to the switch at connection time,
     * to set up the flow table. */
    const struct ofputil_flow_mod *default_flows;
    size_t n_default_flows;
    enum ofputil_protocol usable_protocols;
};

struct lswitch *lswitch_create(struct rconn *, const struct lswitch_config *);
bool lswitch_is_alive(const struct lswitch *);
void lswitch_set_queue(struct lswitch *sw, uint32_t queue);
void lswitch_run(struct lswitch *);
void lswitch_wait(struct lswitch *);
void lswitch_destroy(struct lswitch *);

#endif /* learning-switch.h */
