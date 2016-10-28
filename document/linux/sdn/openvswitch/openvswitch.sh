#!/bin/bash
set -x

start ()
{
	if [ ! -e /usr/etc/openvswitch/conf.db ]
	then
        	ovsdb-tool create /usr/etc/openvswitch/conf.db /usr/share/openvswitch/vswitch.ovsschema
	fi

	modprobe openvswitch
	
	ovsdb-server /usr/etc/openvswitch/conf.db -vconsole:emer -vsyslog:err -vfile:info --remote=punix:/var/run/openvswitch/db.sock --private-key=db:Open_vSwitch,SSL,private_key --certificate=db:Open_vSwitch,SSL,certificate --bootstrap-ca-cert=db:Open_vSwitch,SSL,ca_cert --no-chdir --log-file=/var/log/openvswitch/ovsdb-server.log --pidfile=/var/run/openvswitch/ovsdb-server.pid --detach --monitor

	ovs-vswitchd unix:/var/run/openvswitch/db.sock -vconsole:emer -vsyslog:err -vfile:info --mlockall --no-chdir --log-file=/var/log/openvswitch/ovs-vswitchd.log --pidfile=/var/run/openvswitch/ovs-vswitchd.pid --detach --monitor
}


stop ()
{
	kill -s 9 `cd /var/run/openvswitch && cat ovsdb-server.pid ovs-vswitchd.pid`	
	cat /proc/modules | grep openvswitch > /dev/null
	if [ $? -eq 0 ]; then
    		rmmod openvswitch
	fi
}

case $1 in
    start)
        start
        ;;
    stop)
        stop
        ;;
    restart)
        stop
	start 
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}" >&2
        exit 1
        ;;
esac

exit 0
