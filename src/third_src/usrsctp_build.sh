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
function GenerateCmakeFile()
{
#   mkdir -p build
    CmakeFile="$2/ToolChain.cmake"
    CurPwd=$PWD
    
    cd ../../../build/$1
    if [ -e "third_lib" ]; then
        echo "third_lib exit"
    else
        mkdir third_lib
    fi  
    cd third_lib
    if [ -e "usrsctp" ]; then
        echo "usrsctp exit"
    else
        mkdir usrsctp
    fi  
    cd usrsctp  
    CmakePrefix=$PWD
    
    cd $CurPwd
    echo "SET(CMAKE_SYSTEM_NAME \"Linux\")" > $CmakeFile
    if [ $1 == x86 ]; then
        echo "SET(CMAKE_C_COMPILER \"gcc\")" >> $CmakeFile  
        echo "SET(CMAKE_CXX_COMPILER \"g++\")" >> $CmakeFile    
    else
        echo "SET(CMAKE_C_COMPILER \"$1-gcc\")" >> $CmakeFile
        echo "SET(CMAKE_CXX_COMPILER \"$1-g++\")" >> $CmakeFile     
    fi
    echo "SET(CMAKE_INSTALL_PREFIX \"$CmakePrefix\")" >> $CmakeFile     
    echo "SET(CMAKE_ToolChain \"$1\")" >> $CmakeFile
}
function BuildLib()
{
    echo -e "Start building usrsctp..."
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
    #mkdir $OutputPath/lib
    
    GenerateCmakeFile $1 $OutputPath    
    cd $OutputPath
#在cmakelist.txt中set CMAKE_TOOLCHAIN_FILE    变量无用，所以只好外面传入(可能是该库分两个目录需要环境变量的缘故)
#加了CMAKE_TOOLCHAIN_FILE，即使该变量指向的文件已经包含了 CMAKE_INSTALL_PREFIX ，但cmake不去用，所以还要加CMAKE_INSTALL_PREFIX
    cmake -DCMAKE_TOOLCHAIN_FILE=build/ToolChain.cmake -DCMAKE_INSTALL_PREFIX=. ..
    if [ -e "Makefile" ]; then  
        make clean
        make -j4 > /dev/null
        if [ $? == 0 ]; then
            echo "make success! "
        else
            echo "make failed! "
            exit -1
        fi
        make install
    else
        echo "Makefile generated failed! "
        exit -1
    fi
    cd ..
}

function CopyLib()
{
#   CurPwd = $PWD
    CurPwd=$PWD
    cd $1
    if [ -e "third_lib" ]; then
        echo "third_lib exit"
    else
        mkdir third_lib
    fi
    
    cd third_lib
    if [ -e "usrsctp" ]; then
        echo "usrsctp exit"
    else
        mkdir usrsctp
    fi
    
    cd usrsctp
    cp $CurPwd/build/lib . -rf
    

    cp $CurPwd/build/include . -rf

}

if [ $# == 0 ]; then
    PrintUsage
    exit -1
else
#   GenerateCmakeFile $1
    BuildLib $1
#   CopyLib ../../../build/$1
fi




