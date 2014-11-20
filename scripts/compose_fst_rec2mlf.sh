#!/bin/bash

# The COMP_OUT_DIR folder contains one folder for each query (term), in which there are .rec files, one for each test utterance

if [ $# -ne 1 ]; then
	echo "Usage: $0 COMP_OUT_DIR"
	exit 1
fi

COMP_OUT_DIR=$1
MLF_OUT_DIR=./mlf
mkdir -p $MLF_OUT_DIR

SCORE_TYPE=logadd # logadd | best

SCORE_COLUMN=-1
if [ $SCORE_TYPE == "logadd" ]; then
	SCORE_COLUMN=4
elif [ $SCORE_TYPE == "best" ]; then
	SCORE_COLUMN=5
else
	echo "ERROR: Unknown score type '$SCORE_TYPE'" >&2
	exit 1
fi

filelist=$(ls $(ls -d $COMP_OUT_DIR/* | head -1)/ | sed 's/\..*$//' | sort -u)
mlf=$MLF_OUT_DIR/`basename $COMP_OUT_DIR`_$SCORE_TYPE.mlf
(
echo "#!MLF!#"
for file in $filelist; do
	echo "\"${file%.fst}.rec\""
	for rec in `ls $COMP_OUT_DIR/ex$ex/*/$file.rec`; do
		cat $rec \
		| awk '{if(NF>4) {print $1" "$2" "$3" "$'$SCORE_COLUMN';} else {print}}'
	done
	echo "."
done
) > $mlf
