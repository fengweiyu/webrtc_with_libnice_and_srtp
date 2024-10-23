#!/bin/bash

function PrintUsage()
{
    echo -e "Usage:"
    echo -e "./build.sh $ToolChain"
    echo -e "ToolChain: arm-linux/x86/x64/win"
    echo -e "EGG:"
    echo -e "./build.sh arm-linux"
    echo -e " or ./build.sh x86"
}
function GenerateCmakeFile()
{
#   mkdir -p build
    CmakeFile="$2/ToolChain.cmake"
    echo "SET(CMAKE_SYSTEM_NAME \"Linux\")" > $CmakeFile
    if [ $1 == x86 -o $1 == x64 -o $1 == win -o $1 == cygwin ]; then
        echo "SET(CMAKE_C_COMPILER \"gcc\")" >> $CmakeFile  
        echo "SET(CMAKE_CXX_COMPILER \"g++\")" >> $CmakeFile    
    else
        echo "SET(CMAKE_C_COMPILER \"$1-gcc\")" >> $CmakeFile
        echo "SET(CMAKE_CXX_COMPILER \"$1-g++\")" >> $CmakeFile
    fi
}
function BuildLib()
{
    echo -e "Start building $1 MediaConvert..."
    OutputPath="./build"
    if [ -e "$OutputPath" ]; then
        rm $OutputPath -rf
#防止切换平台编译时由于平台不对应报错，所以删除build重新建立       
#       if [ -e "$OutputPath/lib" ]; then
#           echo "/build/lib exit"
#       else
#           mkdir $OutputPath/lib
#       fi
#   else
#       mkdir $OutputPath
#       mkdir $OutputPath/lib
    fi  
    mkdir $OutputPath
    mkdir $OutputPath/lib
    
    GenerateCmakeFile $1 $OutputPath    
    cd $OutputPath
    if [ $1 == win ]; then
        cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER_WORKS=TRUE -DCMAKE_CXX_COMPILER_WORKS=TRUE
    else
        cmake ..
    fi
    
    if [ -e "Makefile" ]; then  
        make clean
        make -j4 > /dev/null
        if [ $? == 0 ]; then
            echo "make success! "
        else
            echo "make failed! "
            exit -1
        fi
    else
        echo "Makefile generated failed! "
        exit -1
    fi
    cd ..
}

function CopyExe()
{
#   CurPwd = $PWD
    CurPwd=$PWD
    
    if [ -e "$1" ]; then
        echo "$1 exit"
    else
        mkdir $1
    fi
    cd $1
    
    if [ -e "media" ]; then
        echo "media exit"
    else
        mkdir media
    fi
    
    cd media
    if [ -e "bin" ]; then
        echo "bin exit"
    else
        mkdir bin
    fi
    
    cd bin
    
    cp $CurPwd/build/MediaConvert .

#由于对外头文件又依赖内部头文件，所以要拷贝，暂时这么处理后续优化   
#    cp $CurPwd/*.h .
}

if [ $# == 0 ]; then
    PrintUsage
    exit -1
else
#   GenerateCmakeFile $1
    BuildLib $1
    CopyExe $2
fi




