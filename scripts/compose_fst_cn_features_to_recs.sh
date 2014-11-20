#!/bin/bash

if [ $# -ne 1 ]; then
	echo "Usage: $0 COMP_DIR" >&2
	exit 1
fi

COMP_DIR=$1

SCORE_COLUMN=dOpgSumW # dScore | dOpgSumW

for i in `find $COMP_DIR -name '*.features'`; do
	cat $i \
	| igawk '@include tables.awk
		NR==1 { parse_table_header(col); next }
		$col["dLenT"]>0 {print $col["dStartT"]" "$col["dEndT"]" "$col["dTerm"]" "$col["'$SCORE_COLUMN'"]}' \
	> ${i%.features}.rec
#	| awk 'NR>1 {print $2" "$3" "$4" "$5}' \
done
