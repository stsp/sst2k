# Makefile for the SST2K project

VERS=$(shell sed <sst.py -n -e '/version=\(.*\)/s//\1/p')

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

pychecker:
	@-pychecker --only --limit 50 sst.py

clean:
	rm -f sst.6 sst.html
	rm -f *.6 MANIFEST index.html SHIPPER.*

release: sst-$(VERS).tar.gz sst.html
	shipper -u -m -t; make clean
