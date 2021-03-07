#!/bin/bash
make clean
make
echo 'run proxy server...'
./proxy &
while true ; do continue ; done
