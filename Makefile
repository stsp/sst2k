#	Makefile for the Super Star Trek game

VERSION=$(shell sed <sst.spec -n -e '/Version: \(.*\)/s//\1/p')

CFLAGS= -O1 -g -Wall -DSSTDOC='"/usr/share/doc/sst/sst.doc"'

.c.o:
	$(CC) $(CFLAGS) -c $<

CFILES= sst.c finish.c reports.c setup.c moving.c battle.c events.c ai.c planets.c io.c sstlinux.c
OFILES= $(CFILES:.c=.o)
HFILES=sst.h
DOCS = README sst-doc.xml sst.xml sst-layer.xsl TODO

# sst.doc and sst.6 are included so target system won't need xmlto
SOURCES= $(CFILES) $(HFILES) $(DOCS) sst.doc sst.6 makehelp.py makefile sst.spec

all: sst sst.doc

ai.o: ai.c sst.h
battle.o: battle.c sst.h
events.o: events.c sst.h
finish.o: finish.c sst.h
io.o: io.c sst.h
moving.o: moving.c sstlinux.h sst.h
planets.o: planets.c sst.h
reports.o: reports.c sst.h
setup.o: setup.c sst.h
sst.o: sst.c sstlinux.h sst.h
sstlinux.o: sstlinux.c sstlinux.h

sst:  $(OFILES)
	gcc  -o sst $(OFILES) -lm -lcurses

$(OFILES):  $(HFILES)

sst.6: sst.xml
	xmlto man sst.xml

sst-doc.txt: sst-doc.xml
	xmlto -m sst-layer.xsl txt sst-doc.xml
sst.doc: sst-doc.txt
	makehelp.py >sst.doc

sst-doc.html: sst-doc.xml
	xmlto xhtml-nochunks sst-doc.xml

install: uninstall sst.6 sst.doc sst-doc.html 
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
sst-$(VERSION).tar.gz: $(SOURCES) sst.6
	ls $(SOURCES) sst.6 | sed s:^:sst-$(VERSION)/: >MANIFEST
	(cd ..; ln -s trunk sst-$(VERSION))
	(cd ..; tar -czvf trunk/sst-$(VERSION).tar.gz `cat trunk/MANIFEST`)
	(cd ..; rm sst-$(VERSION))

dist: sst-$(VERSION).tar.gz

release: sst-$(VERSION).tar.gz sst.html
	shipper; rm -f CHANGES ANNOUNCE* *.6 *.html *.rpm *.lsm MANIFEST

version:
	@echo $(VERSION)