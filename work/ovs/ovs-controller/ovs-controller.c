/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2015 Nicira, Inc.
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

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "command-line.h"
#include "daemon.h"
#include "fatal-signal.h"
#include "ofp-version-opt.h"
#include "openvswitch/ofpbuf.h"
#include "openflow/openflow.h"
#include "poll-loop.h"
#include "rconn.h"
#include "stream-ssl.h"
#include "unixctl.h"
#include "openvswitch/ofp-parse.h"
#include "openvswitch/vconn.h"
#include "openvswitch/vlog.h"
#include "socket-util.h"
#include "openvswitch/ofp-util.h"

#include "traceroute-switch.h"

VLOG_DEFINE_THIS_MODULE(controller);

#define MAX_SWITCHES 16
#define MAX_LISTENERS 16

struct switch_ {
    struct lswitch *lswitch;
};

/* -n, --noflow: Set up flows?  (If not, every packet is processed at the
 * controller.) */
static bool set_up_flows = true;

/* --max-idle: Maximum idle time, in seconds, before flows expire. */
static int max_idle = 60;

/* --with-flows: Flows to send to switch. */
static struct ofputil_flow_mod *default_flows;
static size_t n_default_flows;
static enum ofputil_protocol usable_protocols;

/* --unixctl: Name of unixctl socket, or null to use the default. */
static char *unixctl_path = NULL;

struct in_addr saddr;

static void new_switch(struct switch_ *, struct vconn *);
static void parse_options(int argc, char *argv[]);
OVS_NO_RETURN static void usage(void);

int
main(int argc, char *argv[])
{
    struct unixctl_server *unixctl;
    struct switch_ switches[MAX_SWITCHES];
    struct pvconn *listeners[MAX_LISTENERS];
    int n_switches, n_listeners;
    int retval;
    int i;

    ovs_cmdl_proctitle_init(argc, argv);
    set_program_name(argv[0]);
    service_start(&argc, &argv);
    parse_options(argc, argv);
    fatal_ignore_sigpipe();

    daemon_become_new_user(false);

    if (argc - optind < 1) {
        ovs_fatal(0, "at least one vconn argument required; "
                  "use --help for usage");
    }

    n_switches = n_listeners = 0;
    for (i = optind; i < argc; i++) {
        const char *name = argv[i];
        struct vconn *vconn;

        retval = vconn_open(name, get_allowed_ofp_versions(), DSCP_DEFAULT,
                            &vconn);
        if (!retval) {
            if (n_switches >= MAX_SWITCHES) {
                ovs_fatal(0, "max %d switch connections", n_switches);
            }
            new_switch(&switches[n_switches++], vconn);
            continue;
        } else if (retval == EAFNOSUPPORT) {
            struct pvconn *pvconn;
            retval = pvconn_open(name, get_allowed_ofp_versions(),
                                 DSCP_DEFAULT, &pvconn);
            if (!retval) {
                if (n_listeners >= MAX_LISTENERS) {
                    ovs_fatal(0, "max %d passive connections", n_listeners);
                }

                listeners[n_listeners++] = pvconn;
                VLOG_INFO("listen on: %s as num %d", name, n_listeners);
            }
        }
        if (retval) {
            VLOG_ERR("%s: connect: %s", name, ovs_strerror(retval));
        }
    }
    if (n_switches == 0 && n_listeners == 0) {
        ovs_fatal(0, "no active or passive switch connections");
    }

    daemonize_start(false);

    retval = unixctl_server_create(unixctl_path, &unixctl);
    if (retval) {
        exit(EXIT_FAILURE);
    }

    daemonize_complete();

    while (n_switches > 0 || n_listeners > 0) {
        /* Accept connections on listening vconns. */
        for (i = 0; i < n_listeners && n_switches < MAX_SWITCHES; ) {
            struct vconn *new_vconn;

            retval = pvconn_accept(listeners[i], &new_vconn);
            if (!retval || retval == EAGAIN) {
                if (!retval) {
                    new_switch(&switches[n_switches++], new_vconn);
                }
                i++;
            } else {
                pvconn_close(listeners[i]);
                listeners[i] = listeners[--n_listeners];
            }
        }

        /* Do some switching work.  . */
        for (i = 0; i < n_switches; ) {
            struct switch_ *this = &switches[i];
            lswitch_run(this->lswitch);
            if (lswitch_is_alive(this->lswitch)) {
                i++;
            } else {
                lswitch_destroy(this->lswitch);
                switches[i] = switches[--n_switches];
            }
        }

        unixctl_server_run(unixctl);

        /* Wait for something to happen. */
        if (n_switches < MAX_SWITCHES) {
            for (i = 0; i < n_listeners; i++) {
                pvconn_wait(listeners[i]);
            }
        }
        for (i = 0; i < n_switches; i++) {
            struct switch_ *sw = &switches[i];
            lswitch_wait(sw->lswitch);
        }
        unixctl_server_wait(unixctl);
        poll_block();
    }

    return 0;
}

static void
new_switch(struct switch_ *sw, struct vconn *vconn)
{
    struct lswitch_config cfg;
    struct rconn *rconn;

    rconn = rconn_create(60, 0, DSCP_DEFAULT, get_allowed_ofp_versions());
    rconn_connect_unreliably(rconn, vconn, NULL);

    cfg.max_idle = set_up_flows ? max_idle : -1;
    cfg.default_flows = default_flows;
    cfg.n_default_flows = n_default_flows;
    cfg.usable_protocols = usable_protocols;
    sw->lswitch = lswitch_create(rconn, &cfg);
}

static void
parse_options(int argc, char *argv[])
{
    enum {
        OPT_MAX_IDLE = UCHAR_MAX + 1,
        OPT_SADDR,
        OPT_PEER_CA_CERT,
        OPT_WITH_FLOWS,
        OPT_UNIXCTL,
        VLOG_OPTION_ENUMS,
        DAEMON_OPTION_ENUMS,
        OFP_VERSION_OPTION_ENUMS
    };
    static const struct option long_options[] = {
        {"noflow",      no_argument, NULL, 'n'},
        {"max-idle",    required_argument, NULL, OPT_MAX_IDLE},
        {"saddr",       required_argument, NULL, OPT_SADDR},
        {"with-flows",  required_argument, NULL, OPT_WITH_FLOWS},
        {"unixctl",     required_argument, NULL, OPT_UNIXCTL},
        {"help",        no_argument, NULL, 'h'},
        DAEMON_LONG_OPTIONS,
        OFP_VERSION_LONG_OPTIONS,
        VLOG_LONG_OPTIONS,
        STREAM_SSL_LONG_OPTIONS,
        {"peer-ca-cert", required_argument, NULL, OPT_PEER_CA_CERT},
        {NULL, 0, NULL, 0},
    };
    char *short_options = ovs_cmdl_long_options_to_short_options(long_options);
    bool has_saddr = false;

    for (;;) {
        int indexptr;
        char *error;
        int c;

        c = getopt_long(argc, argv, short_options, long_options, &indexptr);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'n':
            set_up_flows = false;
            break;

        case OPT_MAX_IDLE:
            if (!strcmp(optarg, "permanent")) {
                max_idle = OFP_FLOW_PERMANENT;
            } else {
                max_idle = atoi(optarg);
                if (max_idle < 1 || max_idle > 65535) {
                    ovs_fatal(0, "--max-idle argument must be between 1 and "
                              "65535 or the word 'permanent'");
                }
            }
            break;

        case OPT_SADDR:
            if (inet_aton(optarg, &saddr) == 0) {
                ovs_fatal(0, "fail: Invalid saddr %s", optarg);
            }
            VLOG_INFO("args add saddr: %s\n", optarg);
            has_saddr = true;
            break;

        case OPT_WITH_FLOWS:
            error = parse_ofp_flow_mod_file(optarg, OFPFC_ADD, &default_flows,
                                            &n_default_flows,
                                            &usable_protocols);
            if (error) {
                ovs_fatal(0, "%s", error);
            }
            break;

        case OPT_UNIXCTL:
            unixctl_path = optarg;
            break;

        case 'h':
            usage();

        VLOG_OPTION_HANDLERS
        OFP_VERSION_OPTION_HANDLERS
        DAEMON_OPTION_HANDLERS

        STREAM_SSL_OPTION_HANDLERS

        case OPT_PEER_CA_CERT:
            stream_ssl_set_peer_ca_cert_file(optarg);
            break;

        case '?':
            exit(EXIT_FAILURE);

        default:
            abort();
        }
    }

    if (!has_saddr)
        ovs_fatal(0, "fail No saddr");

    free(short_options);

}

static void
usage(void)
{
    printf("%s: OpenFlow controller\n"
           "usage: %s [OPTIONS] METHOD\n"
           "where METHOD is any OpenFlow connection method.\n",
           program_name, program_name);
    vconn_usage(true, true, false);
    daemon_usage();
    ofp_version_usage();
    vlog_usage();
    printf("\nOther options:\n"
           "  -n, --noflow            pass traffic, but don't add flows\n"
           "  -s, --saddr             gateway fake src addr (must have)\n"
           "  --max-idle=SECS         max idle time for new flows\n"
           "  --with-flows FILE       use the flows from FILE\n"
           "  --unixctl=SOCKET        override default control socket name\n"
           "  -h, --help              display this help message\n"
           "  -V, --version           display version information\n");
    exit(EXIT_SUCCESS);
}
