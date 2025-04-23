#!/bin/sh

case "$1" in
deconfig)
	ifconfig $interface up
	;;
bound)
	ic="ifconfig $interface $ip"
	if [ -n "$subnet" ]; then
		ic="$ic netmask $subnet"
	fi
	if [ -n "$broadcast" ]; then
		ic="$ic broadcast $broadcast"
	fi
	ic="$ic up"
	$ic
	if [ -n "$router" ]; then
		route del default >/dev/null 2>&1
		route del default >/dev/null 2>&1
		route add default gw $router dev $interface
	fi
	>/etc/resolv.conf.$interface
	for i in $dns; do
		echo "nameserver $i" >>/etc/resolv.conf.$interface
	done
	;;
esac

