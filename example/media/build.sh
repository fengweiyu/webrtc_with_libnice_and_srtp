#!/bin/bash

function PrintUsage()
{
    echo -e "Usage:"
    echo -e "./build.sh $ToolChain"
    echo -e "ToolChain: arm-linux/x86"
    echo -e "EGG:"
    echo -e "./build.sh arm-linux"
    echo -e " or ./build.sh x86"
}

if [ $# == 0 ]; then
    PrintUsage
    exit -1
else

    cd src
    sh build.sh $1
    if [ $? -ne 0 ]; then
        exit -1
    fi
    cd ..

    cd demo
    sh build.sh $1
    if [ $? -ne 0 ]; then
        exit -1
    fi
    cd ..

fi




