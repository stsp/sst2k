#	Makefile for the Super Star Trek game

VERS=$(shell sed <sst.spec -n -e '/Version: \(.*\)/s//\1/p')

CFLAGS=     -O -g -DSSTDOC='"/usr/share/doc/sst/sst.doc"'

.c.o:
	$(CC) $(CFLAGS) -c $<

CFILES= sst.c finish.c reports.c setup.c linux.c moving.c battle.c events.c ai.c planets.c
OFILES= $(CFILES:.c=.o)
HFILES=sst.h
DOC = sst-doc.xml sst.xml TODO

# sst.doc and sst.6 are included so target system won't need xmlto
SOURCES= $(CFILES) $(HFILES) $(DOC) sst.doc sst.6 makehelp.py makefile sst.spec

all: sst sst.doc

sst:  $(OFILES)
	gcc  -o sst $(OFILES) -lm

$(OFILES):  $(HFILES)

sst.6: sst.xml
	xmlto man sst.xml

sst-doc.txt: sst-doc.xml
	xmlto --skip-validation txt sst-doc.xml
sst.doc: sst-doc.txt
	makehelp.py >sst.doc

sst-doc.html: sst-doc.xml
	xmlto --skip-validation xhhtml-nochunks sst-doc.xml

install: sst.6 uninstall
	install -m 755 -o 0 -g 0 -d $(ROOT)/usr/bin/
	install -m 755 -o 0 -g 0 sst $(ROOT)/usr/bin/sst
	install -m 755 -o 0 -g 0 -d $(ROOT)/usr/share/man/man6/
	install -m 755 -o 0 -g 0 sst.6 $(ROOT)/usr/share/man/man6/sst.6
	mkdir -p /usr/share/doc/sst/
	install -m 644 -o 0 -g 0 sst.doc $(ROOT)/usr/share/doc/sst/
	install -m 644 -o 0 -g 0 sst-doc.html $(ROOT)/usr/share/doc/sst/index.html

uninstall:
	rm -f ${ROOT}/usr/bin/sst ${ROOT}/usr/share/man/man6/sst.6
	rm -fr ${ROOT}/usr/share/doc/sst/

clean:
	rm -f *.o sst sst-doc.html sst-doc.txt sst.doc

# The "trunk" below assumes this is a Subversion working copy
sst-$(VERS).tar.gz: $(SOURCES) sst.6
	ls $(SOURCES) sst.6 | sed s:^:sst-$(VERS)/: >MANIFEST
	(cd ..; ln -s trunk sst-$(VERS))
	(cd ..; tar -czvf trunk/sst-$(VERS).tar.gz `cat trunk/MANIFEST`)
	(cd ..; rm sst-$(VERS))

dist: sst-$(VERS).tar.gz

release: sst-$(VERS).tar.gz sst.html
	shipper -f; rm -f CHANGES ANNOUNCE* *.6 *.html *.rpm *.lsm MANIFEST
