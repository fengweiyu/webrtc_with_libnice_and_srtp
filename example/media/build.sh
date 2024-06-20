#!/bin/bash
#对于win端，依赖使用powershell打开，并且D:\Program Files\Git\usr\bin中的一些linux命令程序添加到环境变量，并且安装了cmake win版本，和MinGW(用于make编译)
#https://blog.csdn.net/fengyuyeguirenenen/article/details/129165633 https://blog.csdn.net/2401_85015191/article/details/138934826
function PrintUsage()
{
    echo -e "Usage:"
    echo -e "./build.sh $ToolChain $OutPath"
    echo -e "ToolChain: arm-linux/x86/x64/win"
    echo -e "EGG:"
    echo -e "sh build.sh win build"
    echo -e " or sh build.sh x86 build"
}

if [ $# != 2 ]; then
    PrintUsage
    exit -1
else
#CopyPwd=$PWD/build/linux/$1
    CopyPwd=$2
    if [ $1 == win ]; then
        CopyPwd=$PWD/$2/$1
    fi
    
    cd src
    sh build.sh $1 $CopyPwd
    if [ $? -ne 0 ]; then
        exit -1
    fi
    cd ..

    cd demo
    sh build.sh $1 $CopyPwd
    if [ $? -ne 0 ]; then
        exit -1
    fi
    cd ..

fi




