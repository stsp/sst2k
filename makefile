CFLAGS=     -O

.c.o:
	$(CC) $(CFLAGS) -c $<

OFILES=     sst.o finish.o reports.o setup.o linux.o moving.o battle.o events.o ai.o planets.o

HFILES=     sst.h

sstos2.exe:  $(OFILES)
	gcc  -o sst $(OFILES) -lm

        
$(OFILES):  $(HFILES)

