CFLAGS=     -O

.c.o:
	$(CC) $(CFLAGS) -c $<

OFILES=     sst.o finish.o reports.o setup.o linux.o moving.o battle.o events.o ai.o planets.o

HFILES=     sst.h

sst:  $(OFILES)
	gcc  -o sst $(OFILES) -lm

$(OFILES):  $(HFILES)

clean:
	rm -f *.o sst

