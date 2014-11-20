#!/bin/bash
# Converting a confusion network to an FST (for utterances - where we want to search)

if [[ -z "$SAUSAGE_IN" || -z "$FST_OUT" || -z "$FST_SYMBOLS" ]]; then
	echo "ERROR: $0: Some variables are missing!" >&2
	exit 1
fi

if [ ! -d `dirname $FST_OUT` ]; then
	mkdir -p `dirname $FST_OUT`
fi

cat $SAUSAGE_IN \
| FORCE_EPSILON_TRANSITIONS=0 FORCE_EPSILON_TRANSITION_LOGPROB=-200 ./sausage2fst.pl \
| tee ${FST_OUT%.*}.fsttxt \
| fstcompile --arc_type=log --isymbols=$FST_SYMBOLS --osymbols=$FST_SYMBOLS \
| fstarcsort --sort_type=ilabel \
> $FST_OUT

