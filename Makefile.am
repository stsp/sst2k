## Process this file with automake to produce Makefile.in

SUBDIRS = po src doc

EXTRA_DIST = config.rpath po m4 \
	autogen.sh sst.spec ChangeLog.old

maintainer-clean-local:
	rm -f configure
	rm -f aclocal.m4
	rm -f `find . -name Makefile.in`
	rm -f `find . -name *~`
	rm -f src/include/config.h.in

distclean-local:
	rm -rf autom4te*.cache

clean-local:
	rm -rf autom4te*.cache

ACLOCAL_AMFLAGS = -I m4
