# EPC

## Description

EPC, Educational Personal Computer Simulator, is a simulator for use with
[PLASM (Portable Assembler)](https://github.com/outpaddling/plasm) and
[PAPP (Portable Assembly-language Preprocessor)](https://github.com/outpaddling/plasm)
for use in computer architecture and assembly language programming courses.

It supports ECISC, a CPU architecture designed to be very orthogonal and
rich in addressing modes so that it is easy to learn and program, while
providing experience with many features common to commercial architectures.

Support for RISC-V is also in the works.

The simulator provides feedback on program quality in the form of total
clock cycles and instruction cycles, to help students learn to optimize
for execution path.  Other features in the works include reporting cache
hit ratio.

See the included DocBook documentation for details.

## Design and Implementation

The code is organized following basic object-oriented design principals, but
implemented in C to minimize overhead and keep the source code accessible to
scientists who don't have time to master the complexities of C++.

Structures are treated as classes, with accessor macros and mutator functions
provided, so dependent applications and libraries need not access
structure members directly.  Since the C language cannot enforce this, it's
up to application programmers to exercise self-discipline.

For detailed coding standards, see
https://github.com/outpaddling/Coding-Standards/.

## Building and installing

epc is intended to build cleanly in any POSIX environment on any CPU
architecture.  Please don't hesitate to open an issue if you encounter
problems on any Unix-like system.

Primary development is done on FreeBSD with clang, but the code is periodically
tested on Linux, MacOS, NetBSD, and OpenIndiana as well. MS Windows is not
supported, unless using a POSIX environment such as Cygwin or Windows
Subsystem for Linux.

The Makefile is designed to be friendly to package managers, such as
[Debian packages](https://www.debian.org/distrib/packages),
[FreeBSD ports](https://www.freebsd.org/ports/),
[MacPorts](https://www.macports.org/), [pkgsrc](https://pkgsrc.org/), etc.
End users should install via one of these if at all possible.

I maintain a FreeBSD port and a pkgsrc package, which is sufficient to install
cleanly on virtually any POSIX platform.  If you would like to see a
package in another package manager, please consider creating a package
yourself.  This will be one of the easiest packages in the collection and
hence a good vehicle to learn how to create packages.

For an overview of available package managers, see the
[Repology website](https://repology.org/).

### Installing epc on FreeBSD:

FreeBSD is a highly underrated platform for scientific computing, with over
1,900 scientific libraries and applications in the FreeBSD ports collection
(of more than 30,000 total), modern clang compiler, fully-integrated ZFS
file system, and renowned security, performance, and reliability.
FreeBSD has a somewhat well-earned reputation for being difficult to set up
and manage compared to user-friendly systems like [Ubuntu](https://ubuntu.com/).
However, if you're a little bit Unix-savvy, you can very quickly set up a
workstation, laptop, or VM using
[desktop-installer](http://www.acadix.biz/desktop-installer.php).

To install the binary package on FreeBSD:

```
pkg install epc
```

You can just as easily build and install from source.  This is useful for
FreeBSD ports with special build options, for building with non-portable
optimizations such as -march=native, building with debugging info, and for 
[work-in-progress ports](https://github.com/outpaddling/freebsd-ports-wip),
for which binary packages are not maintained.

```
cd /usr/ports/emulators/epc && env CFLAGS='-march=native -O2' make install
``` 

### Installing via pkgsrc

pkgsrc is a cross-platform package manager that works on any Unix-like
platform. It is native to [NetBSD](https://www.netbsd.org/) and well-supported
on [Illumos](https://illumos.org/), [MacOS](https://www.apple.com/macos/),
[RHEL](https://www.redhat.com)/[CentOS](https://www.centos.org/), and
many other Linux distributions.
Using pkgsrc does not require admin privileges.  You can install a pkgsrc
tree in any directory to which you have write access and easily install any
of the nearly 20,000 packages in the collection.

The
[auto-pkgsrc-setup](https://github.com/outpaddling/auto-admin/blob/master/Scripts/auto-pkgsrc-setup)
script will help you install pkgsrc in about 10 minutes.  Just download it
and run

```
sh auto-pkgsrc-setup
```

Then, assuming you selected current packages and the default prefix:

```
# Add pkgsrc to your PATH
source ~/Pkgsrc/pkg/etc/pkgsrc.sh   # Or pkgsrc.csh for csh or tcsh
# Install epc
cd ~/Pkgsrc/emulators/epc
sbmake install clean clean-depends
```

See the pkgsrc documentation for more information.

Community support for pkgsrc is available through the
[pkgsrc-users](http://netbsd.org/mailinglists) mailing list.

### Building epc locally

Below are caveman install instructions for development purposes, not
recommended for regular use.
epc depends on [plasm](https://github.com/outpaddling/plasm).
Install plasm before attempting to build epc.

1. Clone epc and dependencies into sibling directories
2. In each dependency directory and then in epc:

    1. Run "make depend" to update Makefile.depend
    2. Run "./cave-man-install.sh" if present, otherwise "make install"

The default install prefix is ../local.  Clone epc, plasm and dependent
apps into sibling directories so that ../local represents a common path to all
of them.

To facilitate incorporation into package managers, the Makefile respects
standard make/environment variables such as CC, CFLAGS, PREFIX, etc.  

Add-on libraries required for the build, such as plasm, should be found
under `${LOCALBASE}`, which defaults to ../local.
The library, headers, and man pages are installed under
`${DESTDIR}${PREFIX}`.  DESTDIR is empty by default and is primarily used by
package managers to stage installations.  PREFIX defaults to `${LOCALBASE}`.

To install directly to /myprefix, assuming plasm is installed there as well,
using a make variable:

```
make LOCALBASE=/myprefix clean depend install
```

Using an environment variable:

```
# C-shell and derivatives
setenv LOCALBASE /myprefix
make clean depend install

# Bourne shell and derivatives
LOCALBASE=/myprefix
export LOCALBASE
make clean depend install
```

View the Makefile for full details.
