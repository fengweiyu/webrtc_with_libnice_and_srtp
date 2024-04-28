#!/bin/bash

function PrintUsage()
{
    echo -e "Usage:"
    echo -e "./build.sh $ToolChain"
    echo -e "ToolChain: arm-hisiv200-linux/x86"
    echo -e "EGG:"
    echo -e "./build.sh arm-hisiv200-linux"
    echo -e " or ./build.sh x86"
}


if [ $# == 0 ]; then
    PrintUsage
    exit -1
else
    IsServer=$2
    OutputPath="./build"
#build目录是给main执行程序编译用的，同时也是直接提供库，类似make install
#所以内部编译的原则是main的编译脚本(库的使用者)才会依赖build里面的东西，其他模块的编译脚本是不依赖的    
    if [ -e "$OutputPath" ]; then
        if [ -e "$OutputPath/$1" ]; then
            echo "/build/$1 exit"
        else
            mkdir $OutputPath/$1
            if [ -e "$OutputPath/$1/include" ]; then
                echo "/build/$1/include exit"
            else
                mkdir $OutputPath/$1/include
            fi
            if [ -e "$OutputPath/$1/lib" ]; then
                echo "/build/$1/lib exit"
            else
                mkdir $OutputPath/$1/lib
            fi
            if [ -e "$OutputPath/$1/third_lib" ]; then
                echo "/build/$1/third_lib exit"
            else
                mkdir $OutputPath/$1/third_lib
            fi          
        fi
        
    else
        mkdir $OutputPath
        mkdir $OutputPath/$1
        mkdir $OutputPath/$1/include
        mkdir $OutputPath/$1/lib
        mkdir $OutputPath/$1/third_lib
    fi
    
    cd src
    sh build.sh $1
    if [ $? -ne 0 ]; then
        exit -1
    fi
    cd ..
    
    cd example
    if [ "$IsServer" == "Server" ]; then
        cp buildServer.sh build.sh -f
        cp CMakeListsServer.txt CMakeLists.txt -f
        sh build.sh $1
        if [ $? -ne 0 ]; then
            exit -1
        fi
    else
        sh build.sh $1
        if [ $? -ne 0 ]; then
            exit -1
        fi
    fi
    cd ..
    
fi




