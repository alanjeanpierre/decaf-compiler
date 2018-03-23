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
for folder in samples/*;
do
    for file in $folder/*.decaf
    do 
        tests=$((tests + 1))
        solutions/dcc < "$file" > "$file.out" 2>&1
        out="$file.out"
        echo -e -n "$file: "

        d="$(./dcc < $file 2>&1 | diff - $out -q 2>&1)"
        if [ "$d" != "" ]
        then
            echo -e "\e[91mTest fail\e[39m"
            if [ "$long" = true ]
            then
                echo -e "\e[31m$d\e[39m"
            fi
            flag=true      
        else
            if [ "$long" = true ]
            then
                cat "$out"
            fi
            echo -e "\e[92mTest pass\e[39m"  
            pass=$((pass + 1))

        fi
    done
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
