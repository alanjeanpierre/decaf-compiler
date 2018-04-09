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
        echo -e -n "$file: "
        touch tmp.asm tmp.errors tmp2.asm tmp2.errors tmp tmp2
        ./dcc < "$file" > tmp.asm 2>tmp.errors |:
        solutions/dcc < "$file" > tmp2.asm 2>tmp2.errors
        d=""
        if [ $? -ne 0 -o -s tmp.errors ]; then
            # shoudl both have errors
            d="$(diff tmp.errors tmp2.errors -q 2>&1)"
        elif [ "$(cat tmp.asm)" = "" ]; then
            d="Segfault, probably";
        else 
            cat defs.asm >> tmp.asm
            cat defs.asm >> tmp2.asm

            echo -e "-1\nCam" | spim  -trap_file trap.handler -file tmp.asm > tmp 2>&1
            echo -e "-1\nCam" | spim -trap_file trap.handler -file tmp2.asm > tmp2 2>&1
            d="$(diff tmp tmp2 -q 2>&1)"
        fi


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

        rm tmp.asm tmp.errors tmp2.asm tmp2.errors tmp tmp2
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
