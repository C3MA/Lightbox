#!/bin/sh

UNIVERSE=$1
if [ -z $UNIVERSE ];
then
	echo "Usage: $0 <universe>";
	exit 1;
fi

while true;
do
	# handle separatly, when it is called the first time:
	if ["$COMPLETE"==""]; then
		COMPLETE="255";
	else
		COMPLETE="255, $COMPLETE";
	fi
	echo $COMPLETE
	ola_set_dmx -u $UNIVERSE -d $COMPLETE
	sleep 1;
	COMPLETE="30, $COMPLETE"
	ola_set_dmx -u $UNIVERSE -d $COMPLETE
	sleep 1;
	COMPLETE="0, $COMPLETE"
	ola_set_dmx -u $UNIVERSE -d $COMPLETE
	sleep 1;
	echo $COMPLETE
done
