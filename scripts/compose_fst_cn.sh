#!/bin/bash

. ~/bin/args_lib.sh
required_vars FST_WHERE FST_WHAT FST_OUT FST_SYMBOLS TERM REFERENCE_MLF

mkdir -p `dirname $FST_OUT`

FEATURES_OUT=${FST_OUT%.fst}.features

use_pipes=1

if [ $use_pipes == 1 ]; then
	~/projects/FST/fstcompose_spec_symbol/fstcompose_spec_symbol --sort --spec-symbol 2 --spec-symbol-type SIGMA --spec-symbol-in-first-fst $FST_WHAT $FST_WHERE - \
	| fsttopsort \
	| LD_LIBRARY_PATH=/homes/kazi/ifapso/usr_64bit/lib ~/projects/FST/fst_traverser/fst_traverser --symbols $FST_SYMBOLS --term $TERM --reference-mlf $REFERENCE_MLF --utterance-filename `basename ${FST_WHERE%.*}` --features-out $FEATURES_OUT -
else
	~/projects/FST/fstcompose_spec_symbol/fstcompose_spec_symbol --sort --spec-symbol 2 --spec-symbol-type SIGMA --spec-symbol-in-first-fst $FST_WHAT $FST_WHERE $FST_OUT
	fsttopsort $FST_OUT $FST_OUT.topsort
	LD_LIBRARY_PATH=/homes/kazi/ifapso/usr_64bit/lib ~/projects/FST/fst_traverser/fst_traverser --symbols $FST_SYMBOLS --term $TERM --reference-mlf $REFERENCE_MLF --utterance-filename `basename ${FST_WHERE%.*}` --features-out $FEATURES_OUT $FST_OUT.topsort
fi

