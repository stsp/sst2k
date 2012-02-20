# Makefile for the SST2K project

VERS=$(shell sed <sst.py -n -e '/version *= *\(.*\)/s//\1/p')

MANDIR=/usr/share/man/man1
BINDIR=/usr/bin

DOCS    = README COPYING NEWS doc/HACKING doc/sst-doc.xml doc/sst-layer.xsl doc/sst.xml
SOURCES = sst.py Makefile replay doc/makehelp.py control $(DOCS)

all: sst-$(VERS).tar.gz

install: sst.6
	cp sst.py $(BINDIR)
	gzip <sst.6 >$(MANDIR)/sst.6.gz

sst.6: doc/sst.xml
	cd doc; xmlto man sst.xml; mv sst.6 ..

sst.html: doc/sst.xml
	cd doc; xmlto html-nochunks sst.xml; mv sst.html ..

sst-$(VERS).tar.gz: $(SOURCES) sst.6
	mkdir sst-$(VERS)
	cp $(SOURCES) sst-$(VERS)
	tar -czf sst-$(VERS).tar.gz sst-$(VERS)
	rm -fr sst-$(VERS)
	ls -l sst-$(VERS).tar.gz

dist: sst-$(VERS).tar.gz

check: #pylint
	cd test; make --quiet

pychecker:
	@-pychecker --quiet --only --limit 50 sst.py

PYLINTOPTS = --rcfile=/dev/null --reports=n --include-ids=y --disable=C0103,C0111,C0301,C0302,C0321,R0902,R0903,R0911,R0912,R0914,R0915,W0312,W0603
pylint:
	@pylint --output-format=parseable $(PYLINTOPTS) sst.py

clean:
	rm -f sst.6 sst.html
	rm -f *.6 MANIFEST index.html SHIPPER.*

release: sst-$(VERS).tar.gz sst.html
	shipper -u -t; make clean
	cd www; upload
