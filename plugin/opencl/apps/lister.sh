#!/bin/sh
cd -- "$(dirname -- "$0")"

gcc $(pkg-config --cflags OpenCL) -o lister lister.c $(pkg-config --libs OpenCL)

