#!/bin/sh

# Configuration shell script

PRJ_DIR=qw
PRJ_NAME=qw

# gets program version
VERSION=`cut -f2 -d\" VERSION`

# default installation prefix
PREFIX=/usr/local

# installation directory for documents
DOCDIR=""

# drivers
DRIVERS=""
DRV_OBJS=""

# parse arguments
while [ $# -gt 0 ] ; do

    case $1 in
    --without-curses)   WITHOUT_CURSES=1 ;;
    --with-large-file)  WITH_LARGE_FILE=1 ;;
    --help)             CONFIG_HELP=1 ;;

    --mingw32)          CC=i586-mingw32msvc-cc
                        WINDRES=i586-mingw32msvc-windres
                        AR=i586-mingw32msvc-ar
                        ;;

    --prefix)           PREFIX=$2 ; shift ;;
    --prefix=*)         PREFIX=`echo $1 | sed -e 's/--prefix=//'` ;;

    --docdir)           DOCDIR=$2 ; shift ;;
    --docdir=*)         DOCDIR=`echo $1 | sed -e 's/--docdir=//'` ;;

    esac

    shift
done

if [ "$CONFIG_HELP" = "1" ] ; then

    echo "Available options:"
    echo "--without-curses      Disable curses (text) interface detection."
    echo "--with-large-file     Include Large File support (>2GB)."
    echo "--prefix=PREFIX       Installation prefix ($PREFIX)."
    echo "--docdir=DOCDIR       Instalation directory for documentation."
    echo "--mingw32             Build using the mingw32 compiler."
    
    echo
    echo "Environment variables:"
    echo "CC                    C Compiler."
    echo "AR                    Library Archiver."
    echo "CFLAGS                Additional arguments for the C Compiler."
    
    exit 1
fi

if [ "$DOCDIR" = "" ] ; then
    DOCDIR=$PREFIX/share/doc/${PRJ_DIR}
fi

echo "Configuring ${PRJ_NAME}..."

echo "/* automatically created by config.sh - do not modify */" > config.h
echo "# automatically created by config.sh - do not modify" > makefile.opts
> config.ldflags
> config.cflags
> .config.log

# set compiler
if [ "$CC" = "" ] ; then
    CC=cc
    # if CC is unset, try if gcc is available
    which gcc > /dev/null 2>&1

    if [ $? = 0 ] ; then
        CC=gcc
    fi
fi

echo "CC=$CC" >> makefile.opts

# Add CFLAGS to CC
CC="$CC $CFLAGS"

# set archiver
if [ "$AR" = "" ] ; then
    AR=ar
fi

echo "AR=$AR" >> makefile.opts

# add version
cat VERSION >> config.h

# add installation prefix
echo "#define CONFOPT_PREFIX \"$PREFIX\"" >> config.h

if [ "$WITH_LARGE_FILE" = 1 ] ; then
    echo "#define _FILE_OFFSET_BITS 64" >> config.h
fi

#########################################################

# configuration directives

# CFLAGS
if [ -z "$CFLAGS" ] ; then
    CFLAGS="-g -Wall"
fi

echo -n "Testing if C compiler supports ${CFLAGS}... "
echo "int main(int argc, char *argv[]) { return 0; }" > .tmp.c

$CC .tmp.c -o .tmp.o 2>> .config.log

if [ $? = 0 ] ; then
    echo "OK"
else
    echo "No; resetting to defaults"
    CFLAGS=""
fi

echo "CFLAGS=$CFLAGS" >> makefile.opts

# Add CFLAGS to CC
CC="$CC $CFLAGS"


# Win32
echo -n "Testing for win32... "
if [ "$WITHOUT_WIN32" = "1" ] ; then
    echo "Disabled by user"
else
    echo "#include <windows.h>" > .tmp.c
    echo "#include <commctrl.h>" >> .tmp.c
    echo "int CALLBACK WinMain(HINSTANCE h, HINSTANCE p, LPSTR c, int m)" >> .tmp.c
    echo "{ return 0; }" >> .tmp.c

    TMP_LDFLAGS="-lws2_32"
    $CC .tmp.c $TMP_LDFLAGS -o .tmp.o 2>> .config.log

    if [ $? = 0 ] ; then
        echo "-mwindows -lcomctl32" >> config.ldflags
        echo "#define CONFOPT_WIN32 1" >> config.h
        echo "OK"
        WITHOUT_UNIX_GLOB=1
        WITH_WIN32=1
        DRIVERS="win32 $DRIVERS"
        DRV_OBJS="qw_drv_win32.o $DRV_OBJS"
        echo $TMP_LDFLAGS >> config.ldflags

        echo -n "Testing for getaddrinfo() in winsock... "
        echo "#include <winsock2.h>" > .tmp.c
        echo "#include <ws2tcpip.h>" >> .tmp.c

        echo "int STDCALL WinMain(HINSTANCE h, HINSTANCE p, LPSTR c, int m)" >> .tmp.c
        echo "{ struct addrinfo *res; struct addrinfo hints; " >> .tmp.c
        echo "getaddrinfo(\"google.es\", \"www\", &hints, &res);" >> .tmp.c

        echo "return 0; }" >> .tmp.c

        $CC .tmp.c $TMP_LDFLAGS -o .tmp.o 2>> .config.log

        if [ $? = 0 ] ; then
            echo "OK"
        else
            echo "No"
            echo "#define CONFOPT_WITHOUT_GETADDRINFO 1" >> config.h
        fi
    else
        echo "No"
    fi
fi


# test for curses / ncurses library
echo -n "Testing for ncursesw... "

if [ "$WITHOUT_CURSES" = "1" ] ; then
    echo "Disabled"
else
    echo "#include <curses.h>" > .tmp.c
    echo "int main(void) { initscr(); endwin(); return 0; }" >> .tmp.c

    TMP_CFLAGS="-I/usr/local/include"
    TMP_LDFLAGS="-L/usr/local/lib -lncursesw"

    $CC $TMP_CFLAGS .tmp.c $TMP_LDFLAGS -o .tmp.o 2>> .config.log
    if [ $? = 0 ] ; then
        echo "#define CONFOPT_CURSES 1" >> config.h
        echo $TMP_CFLAGS >> config.cflags
        echo $TMP_LDFLAGS >> config.ldflags
        echo "OK (ncursesw)"
        DRIVERS="curses $DRIVERS"
        DRV_OBJS="qw_drv_curses.o $DRV_OBJS"
        WITHOUT_ANSI=1
    else
        # retry with ncursesw/ncurses.h
        echo "#include <ncursesw/ncurses.h>" > .tmp.c
        echo "int main(void) { initscr(); endwin(); return 0; }" >> .tmp.c

        $CC $TMP_CFLAGS .tmp.c $TMP_LDFLAGS -o .tmp.o 2>> .config.log

        if [ $? = 0 ] ; then
            echo "#define CONFOPT_CURSES 1" >> config.h
            echo "#define CONFOPT_NCURSESW_NCURSES 1" >> config.h
            echo $TMP_CFLAGS >> config.cflags
            echo $TMP_LDFLAGS >> config.ldflags
            echo "OK (ncursesw with ncursesw/ncurses.h)"
            DRIVERS="curses $DRIVERS"
            DRV_OBJS="qw_drv_curses.o $DRV_OBJS"
            WITHOUT_ANSI=1
        else
            echo "No"
            WITHOUT_CURSES=1
        fi
    fi
fi

# ANSI
echo -n "Testing for ANSI terminal support... "

if [ "$WITHOUT_ANSI" = "1" ] ; then
    echo "Disabled"
else
    rm -f .tmp.c
    echo "#include <stdio.h>" >> .tmp.c
    echo "#include <termios.h>" >> .tmp.c
    echo "#include <unistd.h>" >> .tmp.c
    echo "#include <sys/select.h>" >> .tmp.c
    echo "#include <signal.h>" >> .tmp.c
    echo "int main(void) { struct termios o; tcgetattr(0, &o); return 0; }" >> .tmp.c

    TMP_CFLAGS=""
    TMP_LDFLAGS=""

    $CC $TMP_CFLAGS .tmp.c $TMP_LDFLAGS -o .tmp.o 2>> .config.log
    if [ $? = 0 ] ; then
        echo "#define CONFOPT_ANSI 1" >> config.h
        echo $TMP_CFLAGS >> config.cflags
        echo $TMP_LDFLAGS >> config.ldflags
        echo "OK"
        DRIVERS="ansi $DRIVERS"
        DRV_OBJS="qw_drv_ansi.o $DRV_OBJS"
    else
        echo "No"
        WITHOUT_ANSI=1
    fi
fi

# sys/file.h detection
echo -n "Testing for sys/file.h... "
echo "#include <sys/file.h>" > .tmp.c
echo "int main(void) { return(0); }" >> .tmp.c

$CC .tmp.c -o .tmp.o 2>> .config.log

if [ $? = 0 ] ; then
    CONFOPT_SYS_FILE_H=1
    echo "#define CONFOPT_SYS_FILE_H 1" >> config.h
    echo "OK"
else
    echo "No"
fi

if [ -z "$WITH_WIN32" ] ; then
    echo -n "Testing for POSIX threads and semaphores... "
    echo "#include <pthread.h>" > .tmp.c
    echo "#include <semaphore.h>" >> .tmp.c
    echo "void *f(void *p) { return p; } int main(void) { pthread_t t; sem_t s; pthread_create(&t, NULL, f, NULL); sem_init(&s, 0, 0); return 0; }" >> .tmp.c

    TMP_LDFLAGS="-pthread"
    $CC .tmp.c $TMP_LDFLAGS -o .tmp.o 2>> .config.log

    if [ $? = 0 ] ; then
        echo "#define CONFOPT_PTHREADS 1" >> config.h
        echo $TMP_LDFLAGS >> config.ldflags
        WITH_PTHREADS=1
        echo "OK"
    else
        echo "No"
    fi
fi

# test for Grutatxt
echo -n "Testing if Grutatxt is installed... "

DOCS="\$(ADD_DOCS)"

if which grutatxt > /dev/null 2>&1 ; then
    echo "OK"
    echo "GRUTATXT=yes" >> makefile.opts
    DOCS="$DOCS \$(GRUTATXT_DOCS)"
else
    echo "No"
    echo "GRUTATXT=no" >> makefile.opts
fi

# test for mp_doccer
echo -n "Testing if mp_doccer is installed... "
MP_DOCCER=$(which mp_doccer 2>/dev/null || which mp-doccer 2>/dev/null)

if [ $? = 0 ] ; then

    if ${MP_DOCCER} --help | grep grutatxt > /dev/null ; then

        echo "OK"

        echo "MP_DOCCER=yes" >> makefile.opts
        DOCS="$DOCS \$(MP_DOCCER_DOCS)"

        grep GRUTATXT=yes makefile.opts > /dev/null && DOCS="$DOCS \$(G_AND_MP_DOCS)"
    else
        echo "Outdated (No)"
        echo "MP_DOCCER=no" >> makefile.opts
    fi
else
    echo "No"
    echo "MP_DOCCER=no" >> makefile.opts
fi

#########################################################

# final setup

echo "DOCS=$DOCS" >> makefile.opts
echo "VERSION=$VERSION" >> makefile.opts
echo "PREFIX=\$(DESTDIR)$PREFIX" >> makefile.opts
echo "DOCDIR=\$(DESTDIR)$DOCDIR" >> makefile.opts
echo "DRV_OBJS=$DRV_OBJS" >> makefile.opts
echo >> makefile.opts

cat makefile.opts makefile.in > Makefile

if [ -f makefile.depend ] ; then
    cat makefile.depend >> Makefile
fi

#########################################################

# cleanup

rm -f .tmp.c .tmp.o

if [ "$DRIVERS" = "" ] ; then

    echo
    echo "*ERROR* No usable drivers (interfaces) found"
    echo "See the README file for the available options."

    exit 1
fi

echo
echo "Configured drivers:" $DRIVERS
echo
echo Type 'make' to build ${PRJ_NAME}.

# insert driver detection code into qw_avail_drv.h

TRY_DRIVERS="#define TRY_DRIVERS() ("
echo > qw_avail_drv.h
for drv in $DRIVERS ; do
    echo "int ${drv}_drv_detect(qw_core_t *c, int * argc, char *** argv);" >> qw_avail_drv.h
    TRY_DRIVERS="$TRY_DRIVERS ${drv}_drv_detect(c, &argc, &argv) || "
done

echo >> qw_avail_drv.h
echo $TRY_DRIVERS '0 )' >> qw_avail_drv.h

exit 0
