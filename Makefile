#----------------------------------------------------------------------
#
#	Makefile for rcsi
#
#----------------------------------------------------------------------
# $Id: Makefile,v 1.12 2012/12/17 12:15:07 cmb Exp $
#
# $Log: Makefile,v $
# Revision 1.12  2012/12/17 12:15:07  cmb
# Updated version string - had forgotten to do this for 0.5, but done as
# I increment to 0.6, which added functionality to just list filenames
# without any formatting - so they can be used as arguments to other
# programs...
#
# Revision 1.11  1997/12/05 09:10:20  cmb
# Updated makefile - better target, more warnings etc, not separate
# stripping of executable, and updated version number for creating the
# tar files.
#
# Revision 1.10  1994/11/09  16:52:33  cmb
# Changed tar file creation to version 0.2
#
# Revision 1.9  1994/11/02 14:00:05  cmb
# Added manual page to creation of tar file.
#
# Revision 1.8  1994/11/02 13:32:00  cmb
# Tweaked to be more in tune with multiple target architectures.
#
# Revision 1.7  1994/10/30 14:22:17  cmb
# Fixed minor sytax error.
#
# Revision 1.6  1994/10/30  14:21:20  cmb
# This looks nearly ready to distribute.
#
#----------------------------------------------------------------------

TAR	= tar

all:	rcsi

#----------------------------------------------------------------------
#	Actually build the rcsi binary. First target builds an
#	optimised and stripped version, and defaults to using 'gcc'.
#	Later ones are for the various architectures that I have
#	successfully tested 'rcsi' on.

rcsi	: rcsi.c
	gcc -Wall -O2 -Xlinker -s -o rcsi rcsi.c

sunos	: rcsi

solaris	irix osf1	: rcsi.c
	cc -O -o rcsi rcsi.c

#----------------------------------------------------------------------
#	Make a tar file ready for distribution. Use gnutar and the 
#	in-built compression via gzip.

tar	:
	rm -rf rcsi-0.6
	mkdir rcsi-0.6
	cp COPYING README rcsi.c rcsi.1 Makefile rcsi-0.6
	$(TAR) czvf rcsi-0.6.tar.gz rcsi-0.6

#----------------------------------------------------------------------
#	Get rid of all the guff that accumulates...

clean:
	rm -rf rcsi *~ rcsi-0.6 rcsi-0.6.tar.gz
