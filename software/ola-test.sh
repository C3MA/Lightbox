#!/bin/sh

if [ -z $1 ];
then
	echo "Usage: $0 <universe>";
	exit 1;
fi

while true;
do
	ola_set_dmx -u $1 -d 255,0,0
	sleep 1;
	ola_set_dmx -u $1 -d 30,0,0
	sleep 1;
	ola_set_dmx -u $1 -d 0,255,0
	sleep 1;
	ola_set_dmx -u $1 -d 0,30,0
	sleep 1;
	ola_set_dmx -u $1 -d 0,0,255
	sleep 1;
	ola_set_dmx -u $1 -d 0,0,30
	sleep 1;
done
