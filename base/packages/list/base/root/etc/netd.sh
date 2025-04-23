#!/bin/sh

readonly RESOLV_CONF_F=/etc/resolv.conf
readonly RESOLV_CONF_STATIC_F=/etc/resolv.conf.static

case "$1" in
dns)
	shift
	_INTERFACE=$@
	shift
	> $RESOLV_CONF_F 
	for ip in "$@"; do
  		echo "nameserver $ip" >> $RESOLV_CONF_F
	done
	cat $RESOLV_CONF_STATIC_F >> $RESOLV_CONF_F
	;;
esac

