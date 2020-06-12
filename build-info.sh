#!/bin/sh

GIT_REV="unknown"

if [ -d .git ] ; then
    if which git > /dev/null 2>&1 ; then
        GIT_REV=$(git rev-parse HEAD)
        rm -f .build-info.c
    fi
fi

if [ ! -f .build-info.c ] ; then
    echo "char *__build_info_git_rev = \"$GIT_REV\";" > .build-info.c
    echo "char *__build_info_time = __DATE__ \" \" __TIME__;" >> .build-info.c
fi
