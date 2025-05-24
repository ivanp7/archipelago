#!/bin/sh

: ${PREFIX:="/usr/local/bin"}
: ${INTERP:="archi"}

FORMAT=":archipelago:M:16:[archi]\\x00::$PREFIX/$INTERP:"

echo "$FORMAT" > /proc/sys/fs/binfmt_misc/register

