#!/bin/bash

make

long=false
if [ "$1" = "-l" ]
then    
    long=true
fi



flag=false
for file in samples/*.frag 
do
    out="samples/$(basename $file .frag).out"
    echo -e "**********************************"
    echo -e "*** Running $file ***"
    echo -e "**********************************"
    
    if [ "$long" = true ]
    then
        cat "$file"
        echo -e "\n**********************************"
    fi
    d="$(./dcc < $file 2>&1 | diff - $out)"
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
    fi
    echo ""
    echo ""
done

for file in samples/*.decaf
do 
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

    fi
    echo ""
    echo ""
done
    
if [ "$flag" = "true" ]
then
    echo -e "\e[91m***************************"
    echo "Tests failed"
    echo "***************************"
else
    echo -e "\e[92m***************************"
    echo "Tests passed"
    echo "***************************"
fi
