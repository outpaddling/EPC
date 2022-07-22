############################################################################
#
#              Another Programmer's Editor Makefile Template
#
# This is a template Makefile for a simple C or C++ program.
# It is meant to serve as a starting point for creating a portable
# Makefile, suitable for use under ports systems like *BSD ports,
# MacPorts, Gentoo Portage, etc.  It contains examples of most common
# Makefile components you will need.  
#
# For most small projects, you should be able to create a usable
# Makefile from this template by assigning the appropriate values
# to variables like BIN, LIB, etc., and removing everything you
# don't need.
#
# For more complex projects, start with the advanced Makefile.
#
# Variables that are conditionally assigned (with ?=) can be overridden
# on the command line as follows:
#
#       make VAR=value
#
# They can also inheret values from parent Makefiles (as in *BSD ports).
# This allows different systems to use the Makefile without modifications.
# For example, MacPorts installs to /opt/local instead of /usr/local,
# and hence might use the following:
# 
#       make PREFIX=/opt/local
#
# Lastly, they can be overridden by the environment.  I like to add -Wall
# to my CFLAGS for development, without having to put it in the Makefile,
# since it's a gcc specific flag.  Hence, I have the following in my
# .cshrc on gcc-based systems (BSD, Linux, Mac OS X):
# 
#       setenv CFLAGS "-O -Wall -pipe"
#
# Different systems may also use different compilers and keep libraries in
# different locations:
#
#       make CC=gcc CFLAGS=-O2 LFLAGS="-L/usr/X11R6 -lX11"
#
# Variables can also be appended in the Makefile (with +=), so that
# flags specified on the command line can be combined with those in
# the Makefile.
############################################################################

############################################################################
# Executable

BIN     = epc
MAN     = epc.1

############################################################################
# List object files that comprise BIN.

OBJS    = epc.o

############################################################################
# Compile, link, and install options

# Install in /usr/local, unless defined by the parent Makefile, the
# environment, or a command line option such as PREFIX=/opt/local.
PREFIX      ?= ../local
MANPREFIX   ?= ${PREFIX}
MANDIR      ?= ${MANPREFIX}/man

# Where to find local libraries and headers.  For MacPorts, override
# with "make LOCALBASE=/opt/local"
LOCALBASE   ?= ${PREFIX}
DATADIR     ?= ${PREFIX}/share/EPC

############################################################################
# Build flags
# Override with "make CC=gcc", "make CC=icc", etc.
# Do not add non-portable options (such as -Wall) using +=

# Portable defaults.  Can be overridden by mk.conf or command line.
CC          ?= gcc
CFLAGS      = -Wall -O

CXX         ?= g++
CXXFLAGS    = -Wall -O

LD          = ${CC}

CPP         ?= cpp

INCLUDES    += -isystem ${PREFIX}/include -isystem ${LOCALBASE}/include
CFLAGS      += ${INCLUDES}
LFLAGS      += -L${LOCALBASE}/lib -ltwintk -lxtend

############################################################################
# Assume first command in PATH.  Override with full pathnames if necessary.
# E.g. "make INSTALL=/usr/local/bin/ginstall"
# Do not place flags here (e.g. RM = rm -f).  Just provide the command
# and let each usage dictate the flags.

MKDIR   ?= mkdir
INSTALL ?= install
LN      ?= ln
RM      ?= rm
AR      ?= ar
PRINTF  ?= printf
CP      ?= cp

############################################################################
# Standard targets required by ports

all:    ${BIN}

# Link rules
${BIN}: ${OBJS}
	${LD} -o ${BIN} ${OBJS} ${LFLAGS}

############################################################################
# Include dependencies generated by "make depend", if they exist.
# These rules explicitly list dependencies for each object file.
# See "depend" target below.  If Makefile.depend does not exist, use
# generic source compile rules.  These have some limitations, so you
# may prefer to create explicit rules for each target file.  This can
# be done automatically using "cpp -M" or "cpp -MM".  Run "man cpp"
# for more information, or see the "depend" target below.

# Rules generated by "make depend"
include Makefile.depend

############################################################################
# Self-generate dependencies the old-fashioned way

depend:
	rm -f Makefile.depend
	for file in *.c; do \
	    ${CPP} ${INCLUDES} -MM $${file} >> Makefile.depend; \
	    ${PRINTF} "\t\$${CC} -c \$${CFLAGS} $${file}\n\n" >> Makefile.depend; \
	done

############################################################################
# Remove generated files (objs and nroff output from man pages)

clean:
	rm -f ${OBJS} ${BIN} *.nr

# Keep backup files during normal clean, but provide an option to remove them
realclean: clean
	rm -f .*.bak *.bak *.BAK *.gmon core *.core


############################################################################
# Install all target files (binaries, libraries, docs, etc.)

install: all
	${MKDIR} -p ${DESTDIR}${PREFIX}/bin ${DESTDIR}${PREFIX}/man/man1
	${INSTALL} -s -m 0555 ${BIN} ${DESTDIR}${PREFIX}/bin
	${INSTALL} -m 0444 Docs/epc.man ${DESTDIR}${MANPREFIX}/man/man1/${MAN}
	${MKDIR} -p ${DESTDIR}${DATADIR}/include
	${INSTALL} -m 0444 Examples/macros.epc ${DESTDIR}${DATADIR}/include
	${INSTALL} -m 0444 Examples/io.epc ${DESTDIR}${DATADIR}/include
	${MKDIR} -p ${DESTDIR}${DATADIR}/Examples
	${INSTALL} -m 0444 Examples/bench.epc ${DESTDIR}${DATADIR}/Examples
	${INSTALL} -m 0444 Examples/echo.epc ${DESTDIR}${DATADIR}/Examples
	${INSTALL} -m 0444 Examples/float.epc ${DESTDIR}${DATADIR}/Examples
	${INSTALL} -m 0444 Examples/loop.epc ${DESTDIR}${DATADIR}/Examples
	${INSTALL} -m 0444 Examples/hello.epc ${DESTDIR}${DATADIR}/Examples
	${INSTALL} -m 0444 Examples/torture.epc ${DESTDIR}${DATADIR}/Examples

proto:
	cproto ${INCLUDES} *.c > temp.h && mv -f temp.h protos.h

