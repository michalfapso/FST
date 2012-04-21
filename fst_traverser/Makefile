EXEC=fst_traverser
ALL_EXTRA_DEPS = libmlf libkws_scorer
include ../Makefile.global
LDFLAGS += -lmlf -lkws_scorer

.PHONY: libmlf libkws_scorer

libmlf:
	make -C ../../LSE/LSE_2.x/libmlf

libkws_scorer:
	make -C ../../LSE/LSE_2.x/libkws_scorer

