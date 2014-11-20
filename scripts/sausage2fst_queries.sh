#!/bin/bash
# Converting a confusion network to an FST (for queries - what we want to search)

if [[ -z "$SAUSAGE_IN" || -z "$FST_OUT" || -z "$FST_SYMBOLS" ]]; then
	echo "ERROR: $0: Some variables are missing!" >&2
	exit 1
fi

if [ ! -d `dirname $FST_OUT` ]; then
	mkdir -p `dirname $FST_OUT`
fi

cat $SAUSAGE_IN \
| REMOVE_EPSILON_TRANSITIONS=0 FORCE_EPSILON_TRANSITIONS=0 FORCE_EPSILON_TRANSITION_LOGPROB=-200 ./sausage2fst.pl \
| fstcompile --arc_type=log --isymbols=$FST_SYMBOLS --osymbols=$FST_SYMBOLS \
| tee $FST_OUT.time \
| fstproject \
| fstprint --isymbols=$FST_SYMBOLS --osymbols=$FST_SYMBOLS --acceptor=true \
| ,/fst_query_add_sigma_loops.pl --loop-label "!NULL" \
| sort -k1,1n -k2,2n -t' ' \
| tee ${FST_OUT%.*}.fsttxt \
| fstcompile --arc_type=log --isymbols=$FST_SYMBOLS --osymbols=$FST_SYMBOLS \
| fstarcsort --sort_type=olabel \
> $FST_OUT

#fstprint --isymbols=$FST_SYMBOLS --osymbols=$FST_SYMBOLS $FST_OUT.time > $FST_OUT.time.txt


# Optional:
# 
# Generate features for the query. This might be useful for
# training a statisticaL model for confidence score estimation.
#
# ~/projects/FST/fst_query_features_extractor/fst_query_features_extractor --symbols "$FST_SYMBOLS" --features-out $FST_OUT.features $FST_OUT.time
