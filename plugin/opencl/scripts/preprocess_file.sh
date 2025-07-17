#!/bin/sh

: ${CC:="gcc"}

[ "$2" ] && cd -- "$2"
[ -r "$1" ] || exit 1

strip_c_comments ()
{
    sed 's/a/aA/g;s/__/aB/g;s/#/aC/g' "$1" |
        "$CC" -P -E - |
        sed 's/aC/#/g;s/aB/__/g;s/aA/a/g'
}

strip_c_comments "$1" | unifdef -x 2 -D__OPENCL_C_VERSION__ -

