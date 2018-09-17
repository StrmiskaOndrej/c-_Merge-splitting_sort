#!/bin/bash
#
# PRL projekt 2: Minimum Extraction sort
# Created by David Spilka (xspilk00) on 10.3.16.
#

if [ $# != 2 ]; then
	echo "Invalid parameters!"
	exit 1
fi

dd if=/dev/urandom bs=1 count=$1 of=numbers 2> /dev/null
mpic++ --prefix /usr/local/share/OpenMPI -o mss mss.cpp
mpirun --prefix /usr/local/share/OpenMPI -np $2 mss numbers $2
rm mss numbers