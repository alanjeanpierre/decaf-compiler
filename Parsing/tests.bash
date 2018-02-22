#!/bin/bash

#make

long=false
if [ "$1" = "-l" ]
then    
    long=true
fi

pass=0
tests=0


flag=false

for file in samples/*.decaf
do 
    tests=$((tests + 1))
    solutions/dcc < "$file" > "samples/$(basename $file .decaf).out" 2>&1
    out="samples/$(basename $file .decaf).out"
    echo -e "**********************************"
    echo -e "*** Running $file ***"
    echo -e "**********************************"

    if [ "$long" = true ]
    then
        cat "$file"
        echo -e "\n**********************************"
    fi
    d="$(./dcc < $file 2>&1 | diff - $out 2>&1)"
    if [ "$d" != "" ]
    then
        echo -e "\e[91mTest fail\e[39m"
        echo ""
        if [ "$long" = true ]
        then
            echo -e "\e[31m$d\e[39m"
        fi
        flag=true      
    else
        if [ "$long" = true ]
        then
            cat "$out"
            echo -e "\n**********************************"
        fi
        echo -e "\e[92mTest pass\e[39m"  
        pass=$((pass + 1))

    fi
    echo ""
    echo ""
done
    
if [ "$flag" = "true" ]
then
    echo -e "\e[91m***************************"
    echo "$pass / $tests Tests Passed"
    echo "***************************"
else
    echo -e "\e[92m***************************"
    echo "$pass / $tests Tests Passed"
    echo "***************************"
fi
