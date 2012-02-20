# Test-suite makefile for sst

all: regress
	@echo "No diff output is good news."

.SUFFIXES: .log .chk

.log.chk:
	../sst.py -r $< >$@ 2>&1

TESTLOADS := $(shell ls *.log | sed '/.log/s///')
buildregress:
	@for test in $(TESTLOADS); do \
	    if [ ! -f $${test}.tst ] ; \
	    then \
		    echo "Remaking $${test}.chk"; \
		    rm -f $${test}.chk && ../sst.py -r $${test}.log >$${test}.chk 2>&1; \
	    fi \
	done
regress:
	@for test in $(TESTLOADS); \
	do \
	    if [ ! -f $${test}.tst ] ; \
	    then \
		(if echo "Testing $${test}" >&2; \
		../sst.py -r $${test}.log >/tmp/regress 2>&1; \
		then diff -u $${test}.chk /tmp/regress; \
		else echo "*** Nonzero return status on $${test}!"; exit 1; fi;) \
	    fi \
	done
	@rm -f /tmp/regress
