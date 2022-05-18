#!/bin/sh

export LC_ALL=C

get_top_of_version() {
    ovs-vsctl get bridge "$1" protocol | sed -n 's/^.*"\([^"]*\)"]$/\1/p'
}

save_flows () {
    for bridge; do
        of_version=$(get_top_of_version "$bridge")
        : ${of_version:=OpenFlow14}

        printf "ovs-ofctl -O $of_version add-tlv-map %s '" "$bridge"
        ovs-ofctl -O $of_version dump-tlv-map $bridge | \
            awk '/^ 0x/ {if (cnt != 0) printf ","; \
                 cnt++;printf "{class="$1",type="$2",len="$3"}->"$4}'
        echo "'"

        if [ "$of_version" '<' OpenFlow14 ]; then
            echo "ovs-ofctl -O $of_version add-flows $bridge \
                \"$workdir/$bridge.flows.dump\""
        else
            echo "ovs-ofctl -O $of_version --bundle add-flows $bridge \
                \"$workdir/$bridge.flows.dump\""
        fi
        ovs-ofctl -O $of_version dump-flows --no-names --no-stats "$bridge" | \
            sed -e '/NXST_FLOW/d' \
                -e '/OFPST_FLOW/d' \
                -e 's/\(idle\|hard\)_age=[^,]*,//g' \
            > "$workdir/$bridge.flows.dump"
    done
}

workdir=$(mktemp -d)
trap 'rm -rf "$workdir"' EXIT

# Save flows
bridges=$(ovs-vsctl -- --real list-br)
flows=$(save_flows $bridges)

# Restart the database first, since a large database may take a
# while to load, and we want to minimize forwarding disruption.
systemctl --job-mode=ignore-dependencies restart ovsdb-server

# Stop ovs-vswitchd.
systemctl --job-mode=ignore-dependencies stop ovs-vswitchd

# Start vswitchd by asking it to wait till flow restore is finished.
ovs-vsctl --no-wait set open_vswitch . other_config:flow-restore-wait="true"
systemctl --job-mode=ignore-dependencies start ovs-vswitchd

# Restore saved flows and inform vswitchd that we are done.
eval "$flows"
ovs-vsctl --if-exists remove open_vswitch . other_config \
    flow-restore-wait="true"
