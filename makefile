CFLAGS=     -O -g

.c.o:
	$(CC) $(CFLAGS) -c $<

OFILES=     sst.o finish.o reports.o setup.o linux.o moving.o battle.o events.o ai.o planets.o

HFILES=     sst.h

all: sst sst.doc

sst:  $(OFILES)
	gcc  -o sst $(OFILES) -lm

$(OFILES):  $(HFILES)

sst-doc.txt: sst-doc.xml
	xmlto --skip-validation txt sst-doc.xml
sst.doc: sst-doc.txt
	makehelp.py >sst.doc

clean:
	rm -f *.o sst sst-doc.txt sst.doc

