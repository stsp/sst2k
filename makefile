CFLAGS=     -O -g

.c.o:
	$(CC) $(CFLAGS) -c $<

CFILES= sst.c finish.c reports.c setup.c linux.c moving.c battle.c events.c ai.c planets.c
OFILES= $(CFILES:.c=.o)
HFILES=sst.h 

SOURCES= $(CFILES) $(HFILES) sst-doc.xml TODO makehelp.py makefile sst.xml

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

clean:
	rm -f *.o sst sst-doc.txt sst.doc

