AM_CFLAGS = -Wall -Wimplicit -Wunused -Wmissing-prototypes \
    -Wuninitialized -fno-inline -O1 -DDATA_DIR=\"$(pkgdatadir)\" @CFLAGS@

bin_PROGRAMS = sst

sst_SOURCES = \
	ai.c \
	battle.c \
	events.c \
	finish.c \
	moving.c \
	planets.c \
	reports.c \
	setup.c \
	sst.c \
	io.c \
	sstlinux.c

EXTRA_DIST = \
	sst.h \
	sstlinux.h
