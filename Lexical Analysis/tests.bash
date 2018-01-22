#!/bin/bash

make

for file in samples/*.frag 
do
    out="samples/$(basename $file .frag).out"
    echo "**********************************"
    echo "*** Running $file ***"
    echo "**********************************"
    echo "$(./dcc < $file | diff -q - $out)"
    echo ""
    echo ""
done

for file in samples/*.decaf
do 
    out="samples/$(basename $file .decaf).out"
    echo "**********************************"
    echo "*** Running $file ***"
    echo "**********************************"
    echo "$(./dcc < $file | diff -q - $out)"
    echo ""
    echo ""
done
    
