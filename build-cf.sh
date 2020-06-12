#!/bin/sh

echo "#include <wchar.h>"
echo 'wchar_t *qw_def_cf[] = {'
sed -e 's/\\/\\\\/g' | sed -e 's/"/\\"/g' | awk '{print " L\"" $0 "\","}'
echo " NULL"
echo "};"
