#!/bin/sh

grep 'QW_KEY_.*,' qw_key.h | sed 's/,//' | while read const ; do
    id=$(echo $const | sed 's/QW_KEY_//' | tr A-Z a-z)
    printf "    { L\"%s\", %s },\n" $id $const
done
