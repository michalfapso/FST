#!/bin/bash
# Converting an HTK lattice to a confusion network

if [[ -z "$LAT_IN" || -z "$MESH_OUT" ]]; then
	echo "ERROR: LAT_IN, MESH_OUT have to be set!" >&2
	exit 1
fi
echo "LAT_IN='$LAT_IN' MESH_OUT='$MESH_OUT'"

WIPEN=0
LMSCALE=0
ACSCALE=${ACSCALE:=1}
#POSCALE=1000000
POSCALE=1

if [ ! -d "`dirname $MESH_OUT`" ]; then
	mkdir -p "`dirname $MESH_OUT`"
fi

lattice-tool \
	-in-lattice $LAT_IN \
	-write-mesh $MESH_OUT \
	-read-htk \
	-htk-logbase 2.718281828459046 \
	-htk-acscale $ACSCALE \
	-htk-lmscale $LMSCALE \
	-htk-wdpenalty $WIPEN \
	-posterior-scale $POSCALE \
	-acoustic-mesh

