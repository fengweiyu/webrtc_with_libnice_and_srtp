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
    echo "SET(CMAKE_SYSTEM_NAME \"Linux\")" > $CmakeFile
    if [ $1 == x86 ]; then
        echo "SET(CMAKE_C_COMPILER \"gcc\")" >> $CmakeFile  
        echo "SET(CMAKE_CXX_COMPILER \"g++\")" >> $CmakeFile
        echo "SET(CMAKE_ToolChain \"$1\")" >> $CmakeFile        
    else
        echo "SET(CMAKE_C_COMPILER \"$1-gcc\")" >> $CmakeFile
        echo "SET(CMAKE_CXX_COMPILER \"$1-g++\")" >> $CmakeFile
        echo "SET(CMAKE_ToolChain \"$1\")" >> $CmakeFile
    fi
}
function Build()
{
    echo -e "Start building example..."
    OutputPath="./build"
    if [ -e "$OutputPath" ]; then
        rm $OutputPath -rf
#防止切换平台编译时由于平台不对应报错，所以删掉重新建立        
#       echo "/build exit"  
#   else
#       mkdir $OutputPath
    fi  
    mkdir $OutputPath
    
    GenerateCmakeFile $1 $OutputPath    
    cd $OutputPath
    cmake ..
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

function CopyLib()
{
#   CurPwd = $PWD
    CurPwd=$PWD
    cd $1
    
    rm ./third_lib -rf
<<COMMENT
#拷贝固定早就编译好的第三方so库
#暂时没时间优化直接使用所有编译结果也可以，libnice依赖架构，所以拷贝所有的，不调整架构
#加了删除操作第二次及以后编译会变慢但是这样更整洁
    rm ./third_lib/zlib -rf
    cp $CurPwd/../lib/$2/zlib_v1.2.11 ./third_lib/zlib -rf
    rm ./third_lib/libffi -rf
    cp $CurPwd/../lib/$2/libffi_v3.2.1 ./third_lib/libffi -rf
    rm ./third_lib/glib -rf
    cp $CurPwd/../lib/$2/glib_2.48.0 ./third_lib/glib -rf
    
    rm ./third_lib/openssl -rf
    cp $CurPwd/../lib/$2/openssl-1.1.1d ./third_lib/openssl -rf
    rm ./third_lib/libnice -rf
    cp $CurPwd/../lib/$2/libnice_0.1.16 ./third_lib/libnice -rf
    rm ./third_lib/libsrtp -rf
    cp $CurPwd/../lib/$2/libsrtp_v2.3.0 ./third_lib/libsrtp -rf 
#cp两次 libnice下会出现libnice_0.1.16，后续处理    
#拷贝目的保持和源码编译结果一样,后续源码编译就可以不用从lib拷贝(直接删除如下语句)
COMMENT
    cd $CurPwd
}
function CopyEXE()
{
#   CurPwd = $PWD
    CurPwd=$PWD
    cd $1
    cp $CurPwd/build/webrtc .
    cd $CurPwd
    
    if [ $2 == x86 ]; then
        strip -s ./build/webrtc
    else
        $2-strip -s ./build/webrtc
    fi      
    cd $1
    cp $CurPwd/build/webrtc ./webrtc_strip
}
if [ $# == 0 ]; then
    PrintUsage
    exit -1
else

    cd http
    sh build.sh $1
    if [ $? -ne 0 ]; then
        exit -1
    fi
    cd ..
<<COMMENT
    cd peerconnection_client
    sh build.sh $1
    if [ $? -ne 0 ]; then
        exit -1
    fi
    cd ..
COMMENT
    cd webrtc_client
    sh build.sh $1
    if [ $? -ne 0 ]; then
        exit -1
    fi
    cd ..
    
    cd media
    sh build.sh $1
    if [ $? -ne 0 ]; then
        exit -1
    fi
    cd ..   
    
    cd rtp
    sh build.sh $1
    if [ $? -ne 0 ]; then
        exit -1
    fi
    cd ..   
    
    cd base64
    sh build.sh $1
    if [ $? -ne 0 ]; then
        exit -1
    fi
    cd ..
    
    cd net
    sh build.sh $1
    if [ $? -ne 0 ]; then
        exit -1
    fi
    cd ..
    
#   GenerateCmakeFile $1
    CopyLib ../build/$1 $1
    Build $1
    CopyEXE ../build/$1 $1
fi




