#!/bin/sh
# conn-monitor.sh constantly monitors the health of the Internet and controls the Wolf LED2 based on it.
# The logic is as follows:
#  no internet -> LED2 is off
#  internet ok, no vpn -> LED2 blinks @ 1 second interval
#  internet ok, vpn ok -> LED2 is on  
readonly LED=/sys/class/leds/yellow\:debug/brightness
readonly LED_TRIGGER=/sys/class/leds/yellow\:debug/trigger
readonly PING_POLL_SEC=60
readonly VPN=172.20.0.1
readonly INTERNET=1.1.1.1

VPN_CONNECTION=true
INTERNET_CONNECTION=true
while true; do
	# Checking health of the WireGuard connection
	if ping -s4 -c1 -W4 $VPN > /dev/null 2>&1; then
		echo none > $LED_TRIGGER
		echo 1 > $LED

		VPN_CONNECTION=true
		INTERNET_CONNECTION=true
	else 
		# Checking health of the internet connection
		if ping -s4 -c1 -W4 $INTERNET > /dev/null; then
			echo timer > $LED_TRIGGER
			INTERNET_CONNECTION=true
		else
			echo none > $LED_TRIGGER
			echo 0 > $LED
			
			INTERNET_CONNECTION=false
		fi
		
		VPN_CONNECTION=false
	fi
	sleep $PING_POLL_SEC
done

