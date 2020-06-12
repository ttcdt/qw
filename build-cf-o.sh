#!/bin/sh

grep 'QW_OP_.*,' qw_op.h | sed 's/,//' | while read const ; do
    id=$(echo $const | sed 's/QW_OP_//' | tr A-Z a-z)
    printf "    { L\"%s\", %s },\n" $id $const
done
